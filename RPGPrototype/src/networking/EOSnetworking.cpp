#include "networking.hpp"

#include "packet_manager.hpp"

// Libs
#include "eos_auth.h"
#include <eos_connect_types.h>
#include "eos_lobby.h"
#include "eos_p2p.h"
#include "eos_sdk.h"
#include "eos_types.h"

// std
#include <cassert>
#include <iostream>
#include <vector>

// TEMP: TODO: Remove
#include <random>
#include <string>

namespace networking {
	Networking::Networking() {
		chatSocketId = {};
		chatSocketId.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
		strcpy_s(chatSocketId.SocketName, "ChatSocket");
		initalizeNetwork();
		createPlatformHandle();
		startEpicLogin();
	}

	void Networking::initalizeNetwork() {
		EOS_InitializeOptions initOptions = {};
		initOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
		initOptions.ProductName = "Invasant";
		initOptions.ProductVersion = "0.1.0";

		EOS_EResult result = EOS_Initialize(&initOptions);
		assert(result == EOS_EResult::EOS_Success && "Failed to connect to networking");
	}

	void Networking::createPlatformHandle() {
		EOS_Platform_Options platformOptions = {};
		platformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
		platformOptions.ProductId = "8b3a12badbd0423c8f93ade7af0c8972";
		platformOptions.SandboxId = "cc09e301ccbf483fb7c352b90b2f9565";
		platformOptions.DeploymentId = "95b4b546d0bf4da69e6fb309b7c8ee1f";
		platformOptions.ClientCredentials.ClientId = "xyza7891mU8nNGXp6zxi22pZQrtuQQIa";
		platformOptions.ClientCredentials.ClientSecret = "cakA54YBgCPYJZely4WY/85XogFvMvnSCtSniggp8cE";
		platformOptions.CacheDirectory = "C:\\Temp\\eos_cache"; //platformOptions.CacheDirectory = "eos_cache";
		platformOptions.bIsServer = EOS_FALSE;
		platformOptions.EncryptionKey = "0000000000000000000000000000000000000000000000000000000000000000"; // 64-char dummy key
		platformOptions.OverrideCountryCode = nullptr;
		platformOptions.OverrideLocaleCode = nullptr;
		platformOptions.TickBudgetInMilliseconds = 0;
		platformOptions.Flags = 0;
		platformOptions.RTCOptions = nullptr;

		platformHandle = EOS_Platform_Create(&platformOptions);
		assert(platformHandle != nullptr && "Failed to create EOS platform instance");

		connectHandle = EOS_Platform_GetConnectInterface(platformHandle);
		assert(connectHandle != nullptr && "Failed to get EOS Connect interface");

		lobbyHandle = EOS_Platform_GetLobbyInterface(platformHandle);
		assert(lobbyHandle != nullptr && "Failed to get EOS Lobby interface");
	}

	void Networking::startEpicLogin() {
		authHandle = EOS_Platform_GetAuthInterface(platformHandle);
		assert(authHandle != nullptr && "Failed to get EOS Auth interface");

		EOS_Auth_Credentials credentials = {};
		credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
		credentials.Type = EOS_ELoginCredentialType::EOS_LCT_AccountPortal;
		credentials.Id = nullptr;
		credentials.Token = nullptr;

		EOS_Auth_LoginOptions loginOptions = {};
		loginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
		loginOptions.Credentials = &credentials;
		loginOptions.ScopeFlags = EOS_EAuthScopeFlags::EOS_AS_NoFlags;

		EOS_Auth_Login(authHandle, &loginOptions, this, &Networking::onAuthLoginComplete);

		std::cout << "[EOS] Epic login initiated." << std::endl;
	}

	void EOS_CALL Networking::onAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* data) {
		if (data->ResultCode == EOS_EResult::EOS_Success) {
			auto* self = static_cast<Networking*>(data->ClientData);

			std::cout << "[EOS] Auth login success." << std::endl;
			self->epicId = data->LocalUserId;

			EOS_Auth_IdToken* idToken = nullptr;
			EOS_Auth_CopyIdTokenOptions copyOptions = {};
			copyOptions.ApiVersion = EOS_AUTH_COPYIDTOKEN_API_LATEST;
			copyOptions.AccountId = self->epicId;

			EOS_EResult result = EOS_Auth_CopyIdToken(self->authHandle, &copyOptions, &idToken);
			if (result != EOS_EResult::EOS_Success || !idToken) {
				std::cerr << "[EOS] Failed to copy ID token: " << EOS_EResult_ToString(result) << std::endl;
				return;
			}

			// Proceed to Connect login with the token
			self->loginWithEpicToken(idToken->JsonWebToken);
			EOS_Auth_IdToken_Release(idToken);
		}
		else {
			std::cerr << "[EOS] Auth login failed: " << EOS_EResult_ToString(data->ResultCode) << std::endl;
		}
	}

	void Networking::loginWithEpicToken(const char* token) {
		EOS_Connect_Credentials credentials = {};
		credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
		credentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC_ID_TOKEN;
		credentials.Token = token;
		std::cout << "[EOS] Token length: " << strlen(token) << std::endl;
		std::cout << "[EOS] Token prefix: " << std::string(token).substr(0, 30) << std::endl;
		std::cout << "[EOS] Using Token: " << token << std::endl;
		std::cout << "[EOS] connectHandle valid: " << (connectHandle != nullptr) << std::endl;

		EOS_Connect_LoginOptions loginOptions = {};
		loginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
		loginOptions.Credentials = &credentials;
		loginOptions.UserLoginInfo = nullptr;

		EOS_Connect_Login(connectHandle, &loginOptions, this, &onConnectComplete);
	}

	void EOS_CALL Networking::onConnectComplete(const EOS_Connect_LoginCallbackInfo* data) {
		auto* self = static_cast<Networking*>(data->ClientData);
		self->productUserId = data->LocalUserId;

		if (data->ResultCode == EOS_EResult::EOS_Success) {
			//char buffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
			//int32_t bufferLength = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
			//EOS_EResult result = EOS_ProductUserId_ToString(self->productUserId, buffer, &bufferLength);
			std::cout << "[EOS] Connect login success. PUID: " << std::endl; //<< buffer << std::endl;
			// TODO: make this work dynamically
			char buffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
			int32_t bufferLength = EOS_PRODUCTUSERID_MAX_LENGTH + 1;
			EOS_ProductUserId_ToString(self->productUserId, buffer, &bufferLength);
			std::cout << "[PUID] I'm: " << buffer << std::endl;
			//char buffer[EOS_PRODUCTUSERID_MAX_LENGTH + 1] = {};
			//EOS_EResult result = EOS_ProductUserId_ToString(self->productUserId, buffer, &bufferLength);
			//std::cout << "ToString result: " << EOS_EResult_ToString(result) << std::endl;

		}
		else if (data->ResultCode == EOS_EResult::EOS_InvalidUser) {
			EOS_Connect_CreateUserOptions createUserOptions = {};
			createUserOptions.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
			createUserOptions.ContinuanceToken = data->ContinuanceToken;

			EOS_Connect_CreateUser(self->connectHandle, &createUserOptions, self, &Networking::onCreateUserComplete);
			self->p2pHandle = EOS_Platform_GetP2PInterface(self->platformHandle);
		}
		else {
			std::cerr << "[EOS] Connect login failed: " << EOS_EResult_ToString(data->ResultCode) << std::endl;
		}
	}

	void EOS_CALL Networking::onCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* data) {
		auto* self = static_cast<Networking*>(data->ClientData);

		if (data->ResultCode == EOS_EResult::EOS_Success) {
			std::cout << "[EOS] User created successfully." << std::endl;
			// Now you can use self->connectHandle
		}
		else {
			std::cerr << "[EOS] Failed to create user: " << EOS_EResult_ToString(data->ResultCode) << std::endl;
		}
	}

	void Networking::sendPacket(const std::vector<uint8_t>& buffer, EOS_ProductUserId target) {
		EOS_P2P_SendPacketOptions options = {};
		options.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
		options.LocalUserId = productUserId;
		options.RemoteUserId = target;
		options.SocketId = &chatSocketId;
		options.Channel = 0;
		options.Data = buffer.data();
		options.DataLengthBytes = static_cast<uint32_t>(buffer.size());
		options.bAllowDelayedDelivery = EOS_TRUE;

		EOS_EResult result = EOS_P2P_SendPacket(p2pHandle, &options);
		std::cout << "[P2P] Sent " << buffer.size() << " bytes, result: " << EOS_EResult_ToString(result) << std::endl;
	}

	void Networking::sendChatMessage(const std::string& text, EOS_ProductUserId targetUserId) {
		ChatPacket packet{ text };
		std::vector<uint8_t> buffer;
		packet.serialize(buffer);
		sendPacket(buffer, targetUserId); // Wraps EOS_P2P_SendPacket
	}

	void Networking::pollIncomingPackets() {
		uint8_t recvBuffer[1024];
		EOS_P2P_ReceivePacketOptions recvOptions = {};
		recvOptions.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
		recvOptions.LocalUserId = productUserId;
		recvOptions.MaxDataSizeBytes = sizeof(recvBuffer);

		EOS_ProductUserId sender;
		EOS_P2P_SocketId socketId = {};
		uint8_t channel = 0;
		uint32_t bytesWritten = 0;

		EOS_EResult result = EOS_P2P_ReceivePacket(p2pHandle, &recvOptions, &sender, &socketId, &channel, recvBuffer, &bytesWritten);
		if (result == EOS_EResult::EOS_Success && bytesWritten > 1) {
			PacketType type = static_cast<PacketType>(recvBuffer[0]);
			const uint8_t* payload = recvBuffer + 1;
			size_t payloadLength = bytesWritten - 1;

			switch (type) {
			case PACKET_CHAT: {
				auto packet = ChatPacket::deserialize(payload, payloadLength);
				std::cout << "[Chat] " << packet.message << std::endl;
				// TODO: Display this
				break;
			}
			case PACKET_MOVEMENT: {
				auto move = MovementPacket::deserialize(payload);
				std::cout << "[Movement] x=" << move.x << " y=" << move.y << " z=" << move.z << std::endl;
				// TODO: Update movement
				break;
			}
			default:
				std::cerr << "[P2P] Unknown packet type: " << static_cast<int>(type) << std::endl;
			}
		}
	}

	void Networking::createLobby() {
		EOS_Lobby_CreateLobbyOptions options = {};
		options.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
		options.LocalUserId = productUserId;
		options.MaxLobbyMembers = 4;
		options.PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
		options.bPresenceEnabled = EOS_TRUE;

		EOS_Lobby_CreateLobby(lobbyHandle, &options, this, &onLobbyCreated);
	}

	void EOS_CALL Networking::onLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* data) {
		if (data->ResultCode == EOS_EResult::EOS_Success) {
			std::cout << "[Lobby] Created successfully. Lobby ID: " << data->LobbyId << std::endl;
			// Store LobbyId if needed
		}
		else {
			std::cerr << "[Lobby] Creation failed: " << EOS_EResult_ToString(data->ResultCode) << std::endl;
		}
	}

}