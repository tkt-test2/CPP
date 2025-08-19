#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>
#include <chrono>

std::mutex mtx;
std::vector<int> primes;

bool is_prime(int n) {
    if (n < 2) return false;
    if (n % 2 == 0 && n != 2) return false;
    for (int i = 3; i <= std::sqrt(n); i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

void find_primes(int start, int end) {
    std::vector<int> local_primes;
    for (int i = start; i <= end; i++) {
        if (is_prime(i)) {
            local_primes.push_back(i);
        }
    }

    std::lock_guard<std::mutex> lock(mtx);
    primes.insert(primes.end(), local_primes.begin(), local_primes.end());
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./prime_parallel <N>" << std::endl;
        return 1;
    }

    int N = std::stoi(argv[1]);
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4; // fallback

    std::vector<std::thread> threads;
    int chunk = N / num_threads;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < num_threads; t++) {
        int s = t * chunk + 1;
        int e = (t == num_threads - 1) ? N : (t + 1) * chunk;
        threads.emplace_back(find_primes, s, e);
    }

    for (auto& th : threads) th.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;

    std::cout << "Found " << primes.size() << " primes up to " << N 
              << " in " << diff.count() << " seconds\n";

    // 最初の20個だけ出力
    for (size_t i = 0; i < primes.size() && i < 20; i++) {
        std::cout << primes[i] << " ";
    }
    std::cout << "..." << std::endl;

    return 0;
}
