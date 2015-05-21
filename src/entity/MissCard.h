#ifndef BANG_MISSCARD_H
#define BANG_MISSCARD_H


#include "InstantCard.h"
#include "../Game.h"


/// An instant defensive card. It counters the effect of BangCard.
class MissCard : public InstantCard {
public:
    MissCard(string original_name, string name, int count);
    virtual bool play(Game * game, Player * player, int position, int target);
};


#endif //BANG_MISSCARD_H
