#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(5005);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sock,
             reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) < 0) {
        std::cerr << "Socket binding failed\n";
        close(sock);
        return 1;
    }

    std::cout << "Waiting for detector packets...\n";

    std::vector<uint8_t> buffer(2048);

    for (int i = 0; i < 24; ++i) {
        ssize_t bytes = recv(
            sock, buffer.data(), buffer.size(), 0
        );

        if (bytes < 8) {
            std::cerr << "Invalid or missing packet\n";
            continue;
        }

        uint32_t frame_net;
        uint16_t packet_net;
        uint16_t total_net;

        std::memcpy(&frame_net, buffer.data(), 4);
        std::memcpy(&packet_net, buffer.data() + 4, 2);
        std::memcpy(&total_net, buffer.data() + 6, 2);

        uint32_t frame_id = ntohl(frame_net);
        uint16_t packet_id = ntohs(packet_net);
        uint16_t total_packets = ntohs(total_net);

        std::cout
            << "Frame " << frame_id
            << " | Packet " << packet_id + 1
            << "/" << total_packets
            << " | Payload " << bytes - 8
            << " bytes\n";
    }

    close(sock);
    return 0;
}