#pragma once
#include <chrono>
#include <string>

class GameTimer {
private:
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point pauseTime;

    double totalSeconds;      
    double remainingSeconds;   
    bool isRunning;
    bool isPaused;

public:
    GameTimer();

    void Start(double seconds);  
    void Stop();                  
    void Pause();                 
    void Resume();                
    void Reset();                 
    void Update();               

    double GetRemainingSeconds() const;
    int GetRemainingMinutes() const;
    int GetRemainingSecondsOnly() const;
    std::wstring GetFormattedTime() const;
    bool IsTimeUp() const;
    bool IsRunning() const { return isRunning && !isPaused; }
};
