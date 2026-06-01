#include "Timer.h"
#include <iomanip>
#include <sstream>

GameTimer::GameTimer()
    : totalSeconds(60.0)
    , remainingSeconds(60.0)
    , isRunning(false)
    , isPaused(false) {
}

void GameTimer::Start(double seconds) {
    totalSeconds = seconds;
    remainingSeconds = seconds;
    startTime = std::chrono::steady_clock::now();
    isRunning = true;
    isPaused = false;
}

void GameTimer::Stop() {
    isRunning = false;
    isPaused = false;
}

void GameTimer::Pause() {
    if (isRunning && !isPaused) {
        pauseTime = std::chrono::steady_clock::now();
        isPaused = true;
    }
}

void GameTimer::Resume() {
    if (isRunning && isPaused) {
        auto now = std::chrono::steady_clock::now();
        auto pauseDuration = now - pauseTime;
        startTime += pauseDuration; 
        isPaused = false;
    }
}

void GameTimer::Reset() {
    remainingSeconds = totalSeconds;
    startTime = std::chrono::steady_clock::now();
    isRunning = true;
    isPaused = false;
}

void GameTimer::Update() {
    if (!isRunning || isPaused) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - startTime);

    remainingSeconds = totalSeconds - elapsed.count();

    if (remainingSeconds <= 0.0) {
        remainingSeconds = 0.0;
        isRunning = false; 
    }
}

double GameTimer::GetRemainingSeconds() const {
    return remainingSeconds;
}

int GameTimer::GetRemainingMinutes() const {
    return static_cast<int>(remainingSeconds) / 60;
}

int GameTimer::GetRemainingSecondsOnly() const {
    return static_cast<int>(remainingSeconds) % 60;
}

std::wstring GameTimer::GetFormattedTime() const {
    std::wstringstream ss;
    ss << std::setw(2) << std::setfill(L'0') << GetRemainingMinutes() << L":"
        << std::setw(2) << std::setfill(L'0') << GetRemainingSecondsOnly();
    return ss.str();
}

bool GameTimer::IsTimeUp() const {
    return !isRunning && remainingSeconds <= 0.0;
}