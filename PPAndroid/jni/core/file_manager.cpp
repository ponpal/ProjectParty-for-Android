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
#include "android/log.h"
#include "platform.h"

#define MAP_FILE_NAME "Map.sdl"
#define MAP_FILE_FOUND 1
#define MAP_FILE_NOT_FOUND 0

#define LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "FileIO", __VA_ARGS__))

enum {
	FILE_SENT_MESSAGE     = 0,
	FILE_REMOVE_MESSAGE   = 1,
	FILE_ALL_SENT_MESSAGE = 2
};

static void sendMapFile(const char* dirName, int socket)
{
	std::string filePath = path::buildPath(dirName, MAP_FILE_NAME);
    uint8_t result;
	if(path::assetExists(filePath.c_str()))
	{
		result = MAP_FILE_FOUND;
		send(socket, &result, sizeof(result), 0);
		Resource map = platformLoadExternalResource(filePath.c_str());
        send(socket, map.buffer, map.length, 0);
        delete map.buffer;
	}
	else
	{
		result = MAP_FILE_NOT_FOUND;
		send(socket, &result, sizeof(result), 0);
	}
}

static void receiveFile(SocketStream* stream, const char* fileDirectory)
{
	uint8_t nameBuffer[100];
    uint16_t nameLen = streamReadShort(stream);
    streamReadBytes(stream, nameBuffer, nameLen);
    std::string name((char*)nameBuffer, nameLen);
    LOG("Receiving file: %s", name.c_str());

    int32_t fileSize = streamReadInt(stream);
    LOG("FileSize: %d", fileSize);
    auto filePath = path::buildPath(fileDirectory, name.c_str()).c_str();
    LOG("FilePath: %s", filePath);
    auto file = fopen(filePath, "w");
    if(file == NULL)
        LOG("File is null! Name: %s", filePath);
    while(fileSize != 0)
    {
        auto length = std::min(0xffff, fileSize);
        auto ptr = streamReadInPlace(stream, length);
        fwrite(ptr, sizeof(uint8_t), length, file);
        fileSize -= length;
    }
    fflush(file);
    fclose(file);
    LOG("Received file: %s", name.c_str());
}
static void removeFiles(SocketStream* stream, const char* fileDirectory)
{
    uint16_t messageLength = streamReadShort(stream);
	uint8_t nameBuffer[100];
    for(int i = 0; i < messageLength; i++)
    {
        uint16_t nameLen = streamReadShort(stream);
        streamReadBytes(stream, nameBuffer, nameLen);
        std::string name((char*)nameBuffer, nameLen);

        LOG("Removing %s", name.c_str());
        auto filePath = path::buildPath(fileDirectory, name.c_str()).c_str();
        remove(filePath);
    }
}

static void receiveFiles(const char* fileDirectory, int socket)
{
	auto stream = streamCreate(socket, 0xffff);

	while(true)
	{
        auto messageID = streamReadByte(stream);
        LOG("MessageID: %d", messageID);
        if(messageID == FILE_ALL_SENT_MESSAGE)
        {
        	break;
        }
        else if(messageID == FILE_SENT_MESSAGE)
        {
        	receiveFile(stream, fileDirectory);
        }
        else if(messageID == FILE_REMOVE_MESSAGE)
        {
        	removeFiles(stream, fileDirectory);
        }
	}
	streamDestroy(stream);
}

static void* fileTask(void* ptr)
{
	auto config = (ReceiveFileConfig*) ptr;
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(config->ip);
	servaddr.sin_port = htons(config->port);

	auto err = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (err)
	{
		LOG("Could not connect to file server, error is: %d %s", errno, strerror(errno));
	}
	sendMapFile(config->fileDirectory, sockfd);
	receiveFiles(config->fileDirectory, sockfd);
	LOG("All files received!");
	close(sockfd);
	config->isDone = true;
}

void receiveFiles(ReceiveFileConfig* serverConfig)
{
		LOG("Receiving files...");
        pthread_t t;
        pthread_create(&t, nullptr, &fileTask, serverConfig);
}



