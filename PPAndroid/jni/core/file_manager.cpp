/*
 * file_manager.cpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Gustav
 */

#include "file_manager.h"
#include "pthread.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include "errno.h"
#include "unistd.h"
#include "strings.h"
#include "path.h"
#include "socket_stream.h"
#include "platform.h"
#include "assert.h"
#include "remote_debug.h"
#include "sys/stat.h"

#define MAP_FILE_NAME "Map.sdl"
#define MAP_FILE_FOUND 1
#define MAP_FILE_NOT_FOUND 0

enum {
	FILE_SENT_MESSAGE     = 0,
	FILE_REMOVE_MESSAGE   = 1,
	FILE_ALL_SENT_MESSAGE = 2
};

uint32_t taskStatus;

typedef struct
{
	uint32_t 	ip;
	uint16_t 	port;
	std::string	fileDirectory;
} ReceiveFileTask;

static void sendMapFile(const char* dirName, int socket)
{
	RLOGI("Sending Map file! %s", dirName);
	std::string filePath = path::buildPath(dirName, MAP_FILE_NAME);
    uint8_t result;
	if(path::assetExists(filePath.c_str()))
	{
		result = MAP_FILE_FOUND;
		send(socket, &result, sizeof(result), 0);
		Resource map = platformLoadAbsolutePath(filePath.c_str());
        send(socket, map.buffer, map.length, 0);
        platformUnloadResource(map);
	}
	else
	{
		result = MAP_FILE_NOT_FOUND;
		send(socket, &result, sizeof(result), 0);
	}
}

bool receiveFile(SocketStream* stream, const char* fileDirectory, const char* name)
{
    RLOGI("Receiving file: %s", name);

    uint32_t fileSize = streamReadInt(stream);
    RLOGI("FileSize: %d", fileSize);
    RLOGI("FileDir: %s", fileDirectory);

    auto filePath = path::buildPath(fileDirectory, name);
    RLOGI("FilePath: %s", filePath.c_str());
    auto file = fopen(filePath.c_str(), "w");
    if(file == NULL) {
    	RLOGE("File is null! Name: %s", filePath.c_str());
    	char* ptr = nullptr;
    	ptr[0] = ' ';
    	return false;
    }

    while(fileSize != 0)
    {
        auto length = std::min(stream->buffer.capacity, fileSize);
        auto ptr = streamReadInPlace(stream, length);
       	fwrite(ptr, sizeof(uint8_t), length, file);
       	fileSize -= length;
    }

    fflush(file);
    fclose(file);
    RLOGI("Received file: %s", name);

    return true;
}

static void removeFiles(SocketStream* stream, const char* fileDirectory)
{
    uint16_t messageLength = streamReadShort(stream);
	uint8_t nameBuffer[196];
    for(int i = 0; i < messageLength; i++)
    {
        uint16_t nameLen = streamReadShort(stream);
        streamReadBytes(stream, nameBuffer, nameLen);
        std::string name((char*)nameBuffer, nameLen);

        RLOGI("Removing %s", name.c_str());
        auto filePath = path::buildPath(fileDirectory, name.c_str()).c_str();
        remove(filePath);
    }
}

static void receiveFiles(const char* fileDirectory, int socket)
{
	auto stream = streamCreate(socket, 0xffff, INPUT_STREAM);

	while(true)
	{
        auto messageID = streamReadByte(stream);
        RLOGI("MessageID: %d", messageID);
        if(messageID == FILE_ALL_SENT_MESSAGE)
        {
        	break;
        }
        else if(messageID == FILE_SENT_MESSAGE)
        {

        	uint8_t nameBuffer[196];
            uint16_t nameLen = streamReadShort(stream);
            streamReadBytes(stream, nameBuffer, nameLen);
            std::string name((char*)nameBuffer, nameLen);

        	receiveFile(stream, fileDirectory, name.c_str());
        }
        else if(messageID == FILE_REMOVE_MESSAGE)
        {
        	removeFiles(stream, fileDirectory);
        }
        else
        {
        	RLOGI("Unrecognised message ID! %d", messageID);
        	platformExit();
        	goto end;
        }
	}

	end:

	streamDestroy(stream);
	RLOGI("%s", "ReceiveFiles done!");
}

static void createDir(const char* fileDir)
{
	int err = mkdir(fileDir, 0770);
	if (err != 0 && errno != 17)
		RLOGI("Error is: %d %s", errno, strerror(errno));
}

static void* fileTask(void* ptr)
{
	auto task = (ReceiveFileTask*) ptr;
	createDir(task->fileDirectory.c_str());

	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(task->ip);
	servaddr.sin_port = htons(task->port);

	RLOGI("%s", "Connecting");

    struct sockaddr_in myaddr;
    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);


	int err = bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr));
	if(err < 0)
		RLOGE("Could not bind socket, %d %s", errno, strerror(err));
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	err = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	if (err)
	{
		RLOGI("Could not set timeout, error is: %d %s", errno, strerror(errno));
		taskStatus = TASK_FAILURE;
		close(sockfd);
		return nullptr;
	}

	err = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	if (err)
	{
		RLOGI("Could not set receive timeout, error is: %d %s", errno, strerror(errno));
		taskStatus = TASK_FAILURE;
		close(sockfd);
		return nullptr;
	}

	RLOGI("Receiving files from: [PORT: %d, IP:%x] ", (uint32_t)task->port, task->ip);
	err = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (err)
	{
		RLOGI("Could not connect to file server, error is: %d %s", errno, strerror(errno));
		taskStatus = TASK_FAILURE;
		close(sockfd);
		return nullptr;
	}
	RLOGI("%s", "Connected");
	sendMapFile(task->fileDirectory.c_str(), sockfd);
	RLOGI("%s", "Sent map file");

	RLOGI("FileDirectory = %s", task->fileDirectory.c_str());

	receiveFiles(task->fileDirectory.c_str(), sockfd);

	RLOGI("%s", "All files received!");

	close(sockfd);
	taskStatus = TASK_SUCCESS;
	return nullptr;
}

uint32_t receiveFiles(uint32_t ip, uint16_t port, const char* fileDirectory)
{
	ASSERT(taskStatus != TASK_PROCESSING, "Already in the process of receiving files");
	auto task = new ReceiveFileTask();
	task->ip = ip;
	task->port = port;
	task->fileDirectory = fileDirectory;
	taskStatus = TASK_PROCESSING;
    RLOGI("Receiving files from: [PORT: %d, IP:%x] %s", (uint32_t)port, ip, fileDirectory);
    pthread_t t;
    pthread_create(&t, 0, &fileTask, task);
    return taskStatus;
}

int32_t receiveFilesStatus(uint32_t)
{
	return taskStatus;
}
