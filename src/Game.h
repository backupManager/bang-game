#ifndef BANG_GAME_H
#define BANG_GAME_H

#include <vector>
#include <memory>
#include <list>
#include "entity/Player.h"
#include "entity/Card.h"
#include "entity/CharacterCard.h"
#include "entity/RoleCard.h"
#include "entity/PlayableCard.h"


/// A game engine holding card pack, connected players and current game state.
class Game {
private:
    vector<shared_ptr<RoleCard>> roles_;
    vector<shared_ptr<CharacterCard>> characters_;
    vector<shared_ptr<PlayableCard>> cards_;
    list<shared_ptr<PlayableCard>> pack_;
    vector<Player *> players_;
    Player * player_on_turn_;
    Player * me_;

    PlayableCard * pending_card_ = NULL;

    /**
     * Assigns roles to players.
     */
    void assignRoles();

    /**
     * Assigns characters to players.
     */
    void assignCharacters();

    /**
     * Assigns cards to players.
     */
    void assignCards();

    /**
     * Sets players default life points to max value.
     */
    void setLifePoints();

    /**
     * Gets player's position.
     */
    int getPlayerPosition(Player * player) const;
public:
    Game();

    ~Game();

    /**
     * Sets current player and adds her to players list.
     */
    void setMe(Player * player);

    /**
     * Returns current player.
     */
    Player * getMe();

    /**
     * Adds a player to players list.
     */
    void addPlayer(Player * player);

    /**
     * Gets players list.
     */
    vector<Player *> & getPlayers();

    /**
     * Gets number of pending players.
     */
    int getPendingPlayersCount() const;

    /**
     * Sets cards pack.
     */
    void setPack(vector<Card *> cards);

    /**
     * Sets a pending card.
     */
    void setPendingCard(PlayableCard * card);

    /**
     * Returns a pending card.
     */
    PlayableCard * getPendingCard();

    /**
     * Initializes the game. Should be called by server.
     */
    void initGame();

    /**
     * Finds cards in pack by their original names.
     */
    vector<shared_ptr<PlayableCard>> getCardsByNames(vector<string> names) const;

    /**
     * Finds a player by name.
     */
    Player * getPlayer(string name);

    /**
     * Finds a card by name.
     */
    shared_ptr<PlayableCard> getCard(string name);

    /**
     * Gets player on turn.
     */
    Player * getPlayerOnTurn();

    /**
     * Computes a distance between two players.
     */
    int getDistance(Player *player, int target) const;

    /**
     * Updates player info. Should be called by client when received updated info from server.
     */
    bool updatePlayer(string name, unsigned int life, string character, string role, bool on_turn, bool pending);

    /**
     * Updates player's permanent cards. Should be called by client when received updated info from server.
     */
    bool updatePermanentCards(string name, vector<string> cards);

    /**
     * A player draws a card.
     */
    void drawCard(Player * player);

    /**
     * Discards a card from hand.
     * \param player A player that wants to discard a card.
     * yparam position A position of the card in hand.
     */
    bool discardCard(Player * player, int position);

    /**
     * Plays a card.
     */
    bool playCard(Player * player, int position, int target);

    /**
     * Finishes current round and hands control over to the next player.
     */
    void finishRound();

    /**
     * Does not reply to pending card.
     */
    void proceed(Player * player);
};


#endif //BANG_GAME_H
