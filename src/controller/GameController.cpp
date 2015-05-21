#include <iostream>
#include "GameController.h"

using namespace std;

GameController::GameController(Game & game, GameClient & client) : game_(game), client_(client) {
    listener_ = std::bind(&GameController::onStreamEvent, this, std::placeholders::_1);
}

void GameController::updatePlayersInfo() {
    vector<vector<string>> players = client_.getPlayersInfo();
    for (vector<string> player : players) {
        game_.updatePlayer(player[0], stoi(player[1]), player[2], player[3], stoi(player[4]), stoi(player[5]));
    }

    if (game_.getMe()->isPending()) {
        state_ = STATE_PENDING;
    } else if (game_.getPlayerOnTurn() == game_.getMe() && game_.getPendingPlayersCount() == 0) {
        state_ = STATE_ON_TURN;
    } else {
        state_ = STATE_WAIT;
    }
}

void GameController::updateCards() {
    vector<string> names = client_.getCards();
    vector<shared_ptr<PlayableCard>> cards = game_.getCardsByNames(names);
    game_.getMe()->setCards(cards);
}

void GameController::actionInit() {
    updatePlayersInfo();
    updateCards();

    client_.addListener(listener_);
    renderBoard();


    int card_position;

    while (true) {
        string line;
        getline(cin, line);

        if (state_ == STATE_ON_TURN) {
            char c;
            if (scanChar(line, c)) {
                if (toupper(c) == 'Z') {
                    state_ = STATE_PLAY_CARD;
                    renderBoard();
                } else if (toupper(c) == 'U') {
                    if (game_.getMe()->getCards().size() > game_.getMe()->getMaxLife()) {
                        state_ = STATE_DISCARD_CARD;
                        renderBoard();
                    } else {
                        client_.finishRound();
                    }
                }
            }
        } else if (state_ == STATE_DISCARD_CARD) {
            int position;
            if (scanInt(line, position)
                && position >= 0 && position < (int) game_.getMe()->getCards().size()) {
                client_.discardCard(position);
                updateCards();

                if (game_.getMe()->getCards().size() > game_.getMe()->getMaxLife()) {
                    renderBoard();
                } else {
                    state_ = STATE_ON_TURN;
                    client_.finishRound();
                }
            } else {
                renderBoard();
            }
        } else if (state_ == STATE_PLAY_CARD) {
            if (scanInt(line, card_position)
                && card_position >= 0 && card_position < (int) game_.getMe()->getCards().size()) {
                if (game_.getMe()->getCards()[card_position]->isTargetable()) {
                    state_ = STATE_PLAY_TARGET;
                    renderBoard();
                } else {
                    client_.playCard(card_position);
                    state_ = STATE_ON_TURN;
                    updatePlayersInfo();
                    updateCards();
                    renderBoard();
                }
            } else {
                renderBoard();
            }
        } else if (state_ == STATE_PLAY_TARGET) {
            char c;
            int player_position;
            if (scanChar(line, c)
                && (player_position = toupper(c) - 'A') >= 0 && player_position < (int) game_.getPlayers().size()) {
                client_.playCard(card_position, player_position);
                state_ = STATE_ON_TURN;
                updatePlayersInfo();
                updateCards();
                renderBoard();
            } else {
                renderBoard();
            }
        } else if (state_ == STATE_PENDING) {
            char c;
            if (scanChar(line, c)) {
                if (toupper(c) == 'Z') {
                    state_ = STATE_PLAY_CARD;
                    renderBoard();
                } else if (toupper(c) == 'P') {
                    state_ = STATE_WAIT;
                    client_.proceed();
                }
            }
        }
    }
}

void GameController::renderBoard() {
    clearScreen();
    printLogo();

    cout << "Hráči:" << endl;
    vector<Player *> players = game_.getPlayers();
    for (unsigned int i = 0; i < players.size(); i++) {
        Player * player = players[i];
        char c = 'A' + i;
        cout << "[" << c << "] ";

        if (player->isPending()) {
            cout << "? ";
        }

        if (player == game_.getPlayerOnTurn()) {
            cout << "<";
        }
        if (player == game_.getMe()) {
            cout << "{";
        }
        cout << player->getName();
        if (player == game_.getMe()) {
            cout << "}";
        }
        if (player == game_.getPlayerOnTurn()) {
            cout << ">";
        }

        cout << " ~ ";
        if (player->getRole() != NULL) {
            cout << player->getRole()->getName() + " ~ ";
        }
        cout << player->getCharacter()->getName();
        cout << "  \u2665 " << player->getLife();
        cout << endl;
    }

    cout << endl;

    cout << "Karty na ruce:" << endl;
    vector<shared_ptr<PlayableCard>> cards = game_.getMe()->getCards();
    for (unsigned int i = 0; i < cards.size(); i++) {
        cout << "[" << i << "] " << cards[i]->getName() << " (" << cards[i]->getOriginalName() << ")" << endl;
    }

    cout << endl;

    if (last_card_) {
        cout << "Poslední zahraná karta:" << endl;
        cout << last_card_player_ << ": " << last_card_->getName() + " (" << last_card_->getOriginalName() + ")";
        if (last_card_target_ >= 0) {
            cout << " -> " << game_.getPlayers()[last_card_target_]->getName();
        }
        cout << endl << endl;
    }

    if (state_ == STATE_ON_TURN) {
        cout << "[Z]ahrát kartu" << endl;
        cout << "[U]končit kolo" << endl;
    } else if (state_ == STATE_DISCARD_CARD) {
        int left = (game_.getMe()->getCards().size() - game_.getMe()->getMaxLife());
        cout << "Před skončením kola odhoď " << left << (left > 1 ? " karty" : " kartu") << "." << endl;
        cout << "Číslo karty: ";
    } else if (state_ == STATE_PLAY_CARD) {
        cout << "Číslo karty: ";
    } else if (state_ == STATE_PLAY_TARGET) {
        cout << "Kód hráče: ";
    } else if (state_ == STATE_PENDING) {
        cout << "[Z]ahrát kartu" << endl;
        cout << "[P]okračovat" << endl;
    }
}

void GameController::actionRefresh() {
    renderBoard();
}

bool GameController::onStreamEvent(vector<string> event) {
    cout << "stream event:" << event[0] << endl;
    if (event[0] == "NEXT_ROUND") {
        updatePlayersInfo();
        updateCards();
        renderBoard();
    } else if (event[0] == "PLAY_CARD") {
        last_card_player_ = event[1];
        last_card_ = game_.getCard(event[2]);
        last_card_target_ = stoi(event[3]);
        updatePlayersInfo();
        updateCards();
        renderBoard();
    } else if (event[0] == "PROCEED") {
        updatePlayersInfo();
        updateCards();
        renderBoard();
    }
    return true;
}
