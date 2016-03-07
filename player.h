#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <cstdint>

#define PLAYER_VOLUME_MAX   255
#define PLAYER_VOLUME_MIN   192

namespace Player
{
    bool init(void);
    void play(void);
    bool occupied(void);
    void stop(void);
    void disable(void);
    void resume(void);
    bool isPaused(void);
    bool isStopped(void);
    bool isDisabled(void);
    void setVolume(uint8_t vol);
    uint8_t getVolume(void);
};

#endif
