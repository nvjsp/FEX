#include "Common/Config.h"
#include "Common/FEXServerClient.h"

#include <FEXCore/Utils/CompilerDefs.h>
#include <FEXCore/Utils/LogManager.h>
#include <FEXCore/Utils/NetStream.h>

#include <fcntl.h>
#include <filesystem>
#include <linux/limits.h>
#include <unistd.h>
#include <string>
#include <sys/poll.h>
#include <sys/prctl.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <thread>

namespace FEXServerClient {
  static int ServerFD {-1};

  std::string GetServerLockFolder() {
    return FEXCore::Config::GetDataDirectory() + "Server/";
  }

  std::string GetServerLockFile() {
    return GetServerLockFolder() + "Server.lock";
  }

  std::string GetServerRootFSLockFile() {
    return GetServerLockFolder() + "RootFS.lock";
  }

  std::string GetServerSocketFile() {
    return std::filesystem::temp_directory_path() / ".FEXServer.socket";
  }

  int GetServerFD() {
    return ServerFD;
  }

  int ConnectToServer() {
    auto ServerSocketFile = GetServerSocketFile();

    // Create the initial unix socket
    int SocketFD = socket(AF_UNIX, SOCK_STREAM, 0);
    if (SocketFD == -1) {
      return -1;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, ServerSocketFile.data(), std::min(ServerSocketFile.size(), sizeof(addr.sun_path)));

    if (connect(SocketFD, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
      close(SocketFD);
      return -1;
    }

    return SocketFD;
  }

  bool SetupClient(char *InterpreterPath) {
    ServerFD = FEXServerClient::ConnectToAndStartServer(InterpreterPath);
    if (ServerFD == -1) {
      return false;
    }

    std::string RootFSPath = FEXServerClient::RequestRootFSPath(ServerFD);

    // If everything has passed then we can now update the rootfs path
    FEXCore::Config::EraseSet(FEXCore::Config::CONFIG_ROOTFS, RootFSPath);

    return true;
  }

  int ConnectToAndStartServer(char *InterpreterPath) {
    int ServerFD = ConnectToServer();
    if (ServerFD == -1) {
      // Couldn't connect to the server. Start one

      // Open some pipes for letting us know when the server is ready
      int fds[2]{};
      if (pipe2(fds, 0) != 0) {
        LogMan::Msg::EFmt("Couldn't open pipe");
        return -1;
      }

      std::string FEXServerPath = std::filesystem::path(InterpreterPath).parent_path().string() + "/FEXServer";
      // Check if a local FEXServer next to FEXInterpreter exists
      // If it does then it takes priority over the installed one
      if (!std::filesystem::exists(FEXServerPath)) {
        FEXServerPath = "FEXServer";
      }

      pid_t pid = fork();
      if (pid == 0) {
        // Child
        close(fds[0]); // Close read end of pipe

        const char *argv[2];

        argv[0] = FEXServerPath.c_str();
        argv[1] = nullptr;

        if (execvp(argv[0], (char * const*)argv) == -1) {
          // Let the parent know that we couldn't execute for some reason
          uint64_t error{1};
          write(fds[1], &error, sizeof(error));

          // Give a hopefully helpful error message for users
          LogMan::Msg::EFmt("Couldn't execute: {}", argv[0]);
          LogMan::Msg::EFmt("This means the squashFS rootfs won't be mounted.");
          LogMan::Msg::EFmt("Expect errors!");
          // Destroy this fork
          exit(1);
        }

        FEX_UNREACHABLE;
      }
      else {
        // Parent
        // Wait for the child to exit so we can check if it is mounted or not
        close(fds[1]); // Close write end of the pipe

        // Wait for a message from FEXServer
        pollfd PollFD;
        PollFD.fd = fds[0];
        PollFD.events = POLLIN | POLLOUT | POLLRDHUP | POLLERR | POLLHUP | POLLNVAL;

        // Wait for a result on the pipe that isn't EINTR
        while (poll(&PollFD, 1, -1) == -1 && errno == EINTR);

        for (size_t i = 0; i < 5; ++i) {
          ServerFD = ConnectToServer();

          if (ServerFD != -1) {
            break;
          }

          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      }
    }
    return ServerFD;
  }

  /**
   * @name Packet request functions
   * @{ */
  void RequestServerKill(int ServerSocket) {
    FEXServerRequestPacket Req {
      .Header {
        .Type = PacketType::TYPE_KILL,
      },
    };

    write(ServerSocket, &Req, sizeof(Req.BasicRequest));
  }

  int RequestLogFD(int ServerSocket) {
    FEXServerRequestPacket Req {
      .Header {
        .Type = PacketType::TYPE_GET_LOG_FD,
      },
    };

    int Result = write(ServerSocket, &Req, sizeof(Req.BasicRequest));
    if (Result != -1) {
      // Wait for success response with SCM_RIGHTS

      FEXServerResultPacket Res{};
      struct iovec iov {
        .iov_base = &Res,
        .iov_len = sizeof(Res),
      };

      struct msghdr msg {
        .msg_name = nullptr,
        .msg_namelen = 0,
        .msg_iov = &iov,
        .msg_iovlen = 1,
      };

      // Setup the ancillary buffer. This is where we will be getting pipe FDs
      // We only need 4 bytes for the FD
      constexpr size_t CMSG_SIZE = CMSG_SPACE(sizeof(int));
      union AncillaryBuffer {
        struct cmsghdr Header;
        uint8_t Buffer[CMSG_SIZE];
      };
      AncillaryBuffer AncBuf{};

      // Now link to our ancilllary buffer
      msg.msg_control = AncBuf.Buffer;
      msg.msg_controllen = CMSG_SIZE;

      ssize_t DataResult = recvmsg(ServerSocket, &msg, 0);
      if (DataResult > 0) {
        // Now that we have the data, we can extract the FD from the ancillary buffer
        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

        // Do some error checking
        if (cmsg == nullptr ||
            cmsg->cmsg_len != CMSG_LEN(sizeof(int)) ||
            cmsg->cmsg_level != SOL_SOCKET ||
            cmsg->cmsg_type != SCM_RIGHTS) {
          // Couldn't get a socket
        }
        else {
          // Check for Success.
          // If type error was returned then the FEXServer doesn't have a log to pipe in to
          if (Res.Header.Type == PacketType::TYPE_SUCCESS) {
            // Now that we know the cmsg is sane, read the FD
            int NewFD{};
            memcpy(&NewFD, CMSG_DATA(cmsg), sizeof(NewFD));
            return NewFD;
          }
        }
      }
    }

    return -1;
  }

  std::string RequestRootFSPath(int ServerSocket) {
    FEXServerRequestPacket Req {
      .Header {
        .Type = PacketType::TYPE_GET_ROOTFS_PATH,
      },
    };

    int Result = write(ServerSocket, &Req, sizeof(Req.BasicRequest));
    if (Result != -1) {
      // Wait for success response with data
      std::vector<char> Data(PATH_MAX + sizeof(FEXServerResultPacket));

      ssize_t DataResult = recv(ServerSocket, Data.data(), Data.size(), 0);
      if (DataResult >= sizeof(FEXServerResultPacket)) {
        FEXServerResultPacket *ResultPacket = reinterpret_cast<FEXServerResultPacket*>(Data.data());
        if (ResultPacket->Header.Type == PacketType::TYPE_GET_ROOTFS_PATH &&
            ResultPacket->MountPath.Length > 0) {
          return std::string(ResultPacket->MountPath.Mount);
        }
      }
    }

    return {};
  }

  /**  @} */

  /**
   * @name FEX logging through FEXServer
   * @{ */

  void MsgHandler(int FD, LogMan::DebugLevels Level, char const *Message) {
    size_t MsgLen = strlen(Message) + 1;

    Logging::PacketMsg Msg;
    Msg.Header = Logging::FillHeader(Logging::PacketTypes::TYPE_MSG);
    Msg.MessageLength = MsgLen;
    Msg.Level = Level;

    const iovec vec[2] = {
      {
        .iov_base = &Msg,
        .iov_len = sizeof(Msg),
      },
      {
        .iov_base = const_cast<char*>(Message),
        .iov_len = Msg.MessageLength,
      },
    };

    writev(FD, vec, 2);
  }

  void AssertHandler(int FD, char const *Message) {
    MsgHandler(FD, LogMan::DebugLevels::ASSERT, Message);
  }
  /**  @} */
}
