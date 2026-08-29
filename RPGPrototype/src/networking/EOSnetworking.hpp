#pragma once

// Libs
#include "eos_sdk.h"

// Std
#include <string>
#include <vector>

namespace networking {
    class Networking {
    public:
        EOS_EpicAccountId epicId;
        EOS_ProductUserId productUserId;
        EOS_ProductUserId remoteUserId;
        EOS_HPlatform platformHandle;

        Networking();

        // Packets
        void sendChatMessage(const std::string& text, EOS_ProductUserId targetUserId);
        void pollIncomingPackets();

        void createLobby();
        void joinLobby();

    private:
        EOS_HConnect connectHandle;
        EOS_HLobby lobbyHandle;
        EOS_HAuth authHandle;
        EOS_HP2P p2pHandle;

        EOS_P2P_SocketId chatSocketId;
        std::string deviceId;

        void initalizeNetwork();
        void createPlatformHandle();
        void startEpicLogin();
        static void EOS_CALL onAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* data);
        void loginWithEpicToken(const char* token);
        static void EOS_CALL onConnectComplete(const EOS_Connect_LoginCallbackInfo* data);
        static void EOS_CALL onCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* data);
        static void EOS_CALL onLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* data);

        void sendPacket(const std::vector<uint8_t>&buffer, EOS_ProductUserId target);
    };
}