
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <chrono>

constexpr int PORT = 5005;
constexpr int EXPECTED_FRAMES = 3;
constexpr int FRAME_BYTES = 64 * 64 * 2;
constexpr int CHUNK_BYTES = 1024;

struct Frame {
    std::vector<std::vector<uint8_t>> packets;
    std::vector<bool> received;
    int count = 0;

    explicit Frame(int total)
        : packets(total), received(total, false) {}
};

bool save_frame(uint32_t id, const Frame& frame) {
    std::string filename =
        "frame_" + std::to_string(id) + ".raw";

    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot create " << filename << "\n";
        return false;
    }

    for (const auto& packet : frame.packets) {
        file.write(
            reinterpret_cast<const char*>(packet.data()),
            static_cast<std::streamsize>(packet.size())
        );
    }

    if (!file) {
        std::cerr << "Failed writing " << filename << "\n";
        return false;
    }

    std::cout << "Saved " << filename
              << " (" << FRAME_BYTES << " bytes)\n";
    return true;
}

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::perror("socket");
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sock, reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) < 0) {
        std::perror("bind");
        close(sock);
        return 1;
    }

    // Avoid waiting forever when packets are missing.
    timeval timeout{};
    timeout.tv_sec = 30;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    std::map<uint32_t, Frame> frames;
    std::vector<uint8_t> buffer(2048);

    int completed = 0;

    std::cout << "Waiting for detector frames...\n";

//  auto start_time = std::chrono::steady_clock::now();
    bool timing_started = false;
    std::chrono::steady_clock::time_point start_time;

    while (completed < EXPECTED_FRAMES) {
        ssize_t bytes = recv(
            sock, buffer.data(), buffer.size(), 0
        );

        if (bytes < 0) {
            std::cout << "Receive timeout or error.\n";
            break;
        }

        if (bytes < 8) {
            std::cerr << "Ignoring short packet\n";
            continue;
        }

        uint32_t frame_net;
        uint16_t index_net;
        uint16_t total_net;

        std::memcpy(&frame_net, buffer.data(), 4);
        std::memcpy(&index_net, buffer.data() + 4, 2);
        std::memcpy(&total_net, buffer.data() + 6, 2);

        uint32_t frame_id = ntohl(frame_net);
        uint16_t index = ntohs(index_net);
        uint16_t total = ntohs(total_net);

        // Validate this project's known image format.
        if (frame_id >= EXPECTED_FRAMES ||
            total != FRAME_BYTES / CHUNK_BYTES ||
            index >= total ||
            bytes != 8 + CHUNK_BYTES) {
            std::cerr << "Ignoring invalid packet\n";
            continue;
        }
        
        // Start timing after the first valid packet.
        if (!timing_started) {
            start_time = std::chrono::steady_clock::now();
            timing_started = true;
        }

        auto it = frames.try_emplace(frame_id, total).first;
        Frame& frame = it->second;

        if (frame.received[index]) {
            std::cout << "Duplicate packet: frame "
                      << frame_id << ", index " << index << "\n";
            continue;
        }

        frame.packets[index].assign(
            buffer.begin() + 8,
            buffer.begin() + bytes
        );

        frame.received[index] = true;
        ++frame.count;

        std::cout << "Frame " << frame_id
                  << " | Packet " << index + 1
                  << "/" << total << "\n";

        if (frame.count == total) {
            if (save_frame(frame_id, frame)) {
                ++completed;
            }
            frames.erase(it);
        }
    }

    auto end_time = std::chrono::steady_clock::now();

    double elapsed = std::chrono::duration<double>(
        end_time - start_time
    ).count();

    double fps = elapsed > 0 ? completed / elapsed : 0.0;

    double throughput = elapsed > 0
        ? (completed * FRAME_BYTES) / (elapsed * 1'000'000.0)
        : 0.0;

    std::cout << "\nPerformance Statistics\n";
    std::cout << "Elapsed time: " << elapsed << " seconds\n";
    std::cout << "Frames per second: " << fps << "\n";
    std::cout << "Image throughput: " << throughput << " MB/s\n";

    for (const auto& [id, frame] : frames) {
        std::cout << "Incomplete frame " << id
                  << ": received " << frame.count
                  << "/" << frame.packets.size()
                  << " packets\n";
    }

    std::cout << "Complete frames saved: "
              << completed << "/" << EXPECTED_FRAMES << "\n";

    close(sock);
    return completed == EXPECTED_FRAMES ? 0 : 1;
}
