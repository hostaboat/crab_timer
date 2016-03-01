#ifndef _PLAYER_H_
#define _PLAYER_H_

namespace Player
{
    bool init(void);
    void play(void);
    void stop(void);
    void resume(void);
    void disable(void);
    bool isPaused(void);
    bool isStopped(void);
    bool isDisabled(void);
};

#endif
