#include <condition_variable>
#include <mutex>
#include <atomic>
#include <chrono>
#include <optional>
#include <limits>

constexpr uint32_t forever = std::numeric_limits<uint32_t>::max();
using ms = std::chrono::milliseconds;

template <typename T>
class Waitable {
private:
    T _t;
    std::condition_variable _cv;
    std::mutex _mut;
    std::atomic<bool> _release {false};

public:
    Waitable(const T& initial_value = {}){
        _t = initial_value;
    }

    ~Waitable(){
        _release = true;
    }

    T get_no_block(){
        std::lock_guard<std::mutex> lck(_mut);
        return _t;
    }

    std::optional<T> get(const uint32_t timeout_ms = forever ){
        std::unique_lock<std::mutex> lck(_mut);
        if(_cv.wait_for(lck, ms(timeout_ms), [this]{ return _release.load(); })){
            return _t;
        }
        return {};
    }

    void update(const T& t){
        std::lock_guard<std::mutex> lck(_mut);
        _t = t;
        _cv.notify_all();
    }

    void release(){
        std::lock_guard<std::mutex> lck(_mut);
        _release = true;
        _cv.notify_all();
    }

};
