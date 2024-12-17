#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "deck.h"
#include "player.h"
#include "strategy.h"

// DECLARE CONSTANTS (WILL BE USER INPUT FROM THE UI [IMPLEMENTED LATER])
#define NUM_DECKS 6
#define H17 1 // 1 for true - TOGGLE FUNCTIONALITY NOT ADDED YET
#define DAS 1 // 1 for true - TOGGLE FUNCTIONALITY NOT ADDED YET
#define LS 1 // 1 for true - TOGGLE FUNCTIONALITY NOT ADDED YET
#define RSA 0 // 0 for false - TOGGLE FUNCTIONALITY NOT ADDED YET
#define ENHC 0 // 0 for false - TOGGLE FUNCTIONALITY NOT ADDED YET
#define MAX_SPLITS 4
#define BJ_PAY 1.5
#define PEN 0.8

#define NUM_SIMULATIONS 200 // Number of shoes to play
#define BANKROLL 10000
#define WAGER 10

// Number of rows & cols for strategy sheet array creation. Not really sure what to do with these.
// They are constants but might change depending on how the strategy sheet is configured
#define STRAT_ROWS 37 // Player hand possibilities
#define STRAT_COLS 10 // Dealer upcards


int main() {
    // Initialize variables & start clock
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    // Initialize output file
    FILE *out = fopen("output.txt", "w");

    // Seed random numbers
    srand(time(NULL));

    // Allocate memory for the strategy sheet 
    // 40 rows (soft, hard, splits, surrenders AND 10 columns [2-A]
    char strategy[STRAT_ROWS][STRAT_COLS];
 
    readStrategySheet("strategy.csv", strategy);

    // Initialize some variables
    Deck deck;
    Player player, dealer;

    int total_num_hands = 0;
    int bankroll = BANKROLL;
    int wager;

    // Initialize players
    initPlayer(&player, BANKROLL, WAGER); // Start with BANKROLL & WAGER parameters
    initPlayer(&dealer, 0, 0);   // Dealer has no bankroll & no wager size

    for (int i=0; i < NUM_SIMULATIONS; i++) { // i = number of shoes  
        // Initialize deck and shuffle
        initDeck(&deck, NUM_DECKS);
        shuffleDeck(&deck);

        // MAIN HAND LOOP
            // While loop goes until the 'cut card' is reached
        while (deck.top <= deck.capacity * PEN) { 
            wager = WAGER; // reset the wager $ amount at the beginning of each hand
            total_num_hands++; // Increment counter (temporary)
            fprintf(out, " --- Shoe #: %d | Hand #: %d | Bankroll: %d\n", i + 1, total_num_hands, bankroll); 

            // Set the surrender boolean, if this gets set to 1 then the player has surrendered and dealer does not act
            int surrender_bool = 0;

            // Clear previous hand data & reinit
            initHand(&player.hand);
            initHand(&dealer.hand);

            // Deal initial hands
            addCardToHand(&player.hand, dealCard(&deck));
            addCardToHand(&player.hand, dealCard(&deck));
            addCardToHand(&dealer.hand, dealCard(&deck));
            
            // Grab the dealer upcard value and store it separately
            int dealerUpcard = getHandValue(&dealer.hand);

            // Deal the dealer another hand (this is to check for naturals)
            addCardToHand(&dealer.hand, dealCard(&deck));

            fprintf(out, "Player's hand value: %d | Soft: %d\n", getHandValue(&player.hand), hasSoftAce(&player.hand));
            fprintf(out, "Dealer's hand value: %d | Upcard: %d\n", getHandValue(&dealer.hand), dealerUpcard);

            // --CHECK FOR NATURALS IF ENHC IS FALSE
            if (ENHC == 0)
            {
                // BOTH PLAYERS HAVE NATURALS. PUSH
                if (getHandValue(&dealer.hand) == 21 && getHandValue(&player.hand) == 21) {
                    freeHand(&dealer.hand);
                    freeHand(&player.hand);
                    continue;
                }
                // Dealer natural
                if (getHandValue(&dealer.hand) == 21) {
                    bankroll -= wager;
                    freeHand(&dealer.hand);
                    freeHand(&player.hand);
                    continue;
                }
                // Player natural - PLAYER WINS
                if (getHandValue(&player.hand) == 21) {
                    bankroll += wager * BJ_PAY;
                    freeHand(&dealer.hand);
                    freeHand(&player.hand);
                    continue;
                }
            }
            
            // While loop to allow the player to act multiple times
            while (1) {
                // THIS IS WHERE THE STRATEGY IMPLEMENTATION GOES
                // Function determinAction() comes from  strategy.c 
                char action = determineAction(&player.hand, dealerUpcard, strategy);

                // Player hits - add 1 card and check for bust
                if (action == 'H')
                {
                    fprintf(out, "Player Hitting\n");
                    addCardToHand(&player.hand, dealCard(&deck));
                    fprintf(out, "Player's NEW hand value: %d\n", getHandValue(&player.hand));
                    if (isBust(&player.hand)) {
                        fprintf(out, "Player busts with value: %d\n", getHandValue(&player.hand));
                        bankroll -= wager;
                        break;
                    }
                }
                // Player stands - No action needed
                else if (action == 'S') {
                    fprintf(out, "Player Standing\n");
                    break;
                }
                // Player splits - create a new hand [ CURRENTLY DOES NOTHING (functions as another stand)]
                else if (action == 'P') {
                    fprintf(out, "split\n");
                    break;
                }
                else if (action == 'D') {
                    fprintf(out, "double/hit\n");
                    if (canDouble(&player.hand))
                    {
                        // Double the wager
                        wager *= 2;
                    }
                    addCardToHand(&player.hand, dealCard(&deck));
                    fprintf(out, "Player's NEW hand value: %d\n", getHandValue(&player.hand));
                    if (isBust(&player.hand)) { // IS THIS IF STATEMENT NECCESARRY? WILL THE PLAYER EVER BUST IN THIS SCENARIO? #1
                        fprintf(out, "Player busts with value: %d\n", getHandValue(&player.hand));
                        bankroll -= wager;
                        break;
                    }
                }
                else if (action == 'T') {
                    fprintf(out, "double/stand\n");
                    if (canDouble(&player.hand))
                    {
                        // Double the wager 
                        wager *= 2;
                        // Add a card
                        addCardToHand(&player.hand, dealCard(&deck));
                        fprintf(out, "Player's NEW hand value: %d\n", getHandValue(&player.hand));
                        if (isBust(&player.hand)) { // IS THIS IF STATEMENT NECCESARRY? WILL THE PLAYER EVER BUST IN THIS SCENARIO? #2
                            fprintf(out, "Player busts with value: %d\n", getHandValue(&player.hand));
                            bankroll -= wager;
                            break;
                        }
                    }
                    // else stand (do nothing)
                    break;
                }
                else if (action == 'X') {
                    fprintf(out, "surrender/stand\n");
                    if (canSurrender(&player.hand) && LS == 1) {
                        // Surrender
                        bankroll -= wager / 2; // subtract half of the wager
                        surrender_bool = 1; 
                        break;
                    }
                    break;
                }
                else if (action == 'Y') {
                    fprintf(out, "surrender/hit\n");
                    if (canSurrender(&player.hand) && LS == 1) {
                        // Surrender
                        bankroll -= wager / 2; // subtract half of the wager
                        surrender_bool = 1; 
                        break;
                    }
                    // Else hit
                    addCardToHand(&player.hand, dealCard(&deck));
                    fprintf(out, "Player's NEW hand value: %d\n", getHandValue(&player.hand));
                    // Check if bust
                    if (isBust(&player.hand)) {
                        fprintf(out, "Player busts with value: %d\n", getHandValue(&player.hand));
                        break;
                    }
                }
                else if (action == 'Z') {
                    fprintf(out, "Surrender/split\n");
                    if (canSurrender(&player.hand) && LS == 1) {
                        // Surrender
                        bankroll -= wager / 2; // subtract half of the wager
                        surrender_bool = 1; 
                        break;
                    }
                    // Else split
                    break;
                }
                else {
                    fprintf(out, "ERROR: INVALID DECISION SELECTED\n");
                    break;
                }
            }
            // Check for bust again after player's turn, return to start of the loop if bust
            // Also check if the player has surrendered and the hand may be canceled
            if (isBust(&player.hand) || surrender_bool == 1) {
                freeHand(&dealer.hand);
                freeHand(&player.hand);
                continue;
            }

            // Dealer's turn to act
            // case 1: H17, dealer hits on soft 17s
            if (H17 == 1)
            {
                while (getHandValue(&dealer.hand) < 17 || ((hasSoftAce(&dealer.hand) == 1) && getHandValue(&dealer.hand) == 17)) {
                addCardToHand(&dealer.hand, dealCard(&deck));
                fprintf(out, "DEALER HITTING - Dealer's NEW hand value: %d\n", getHandValue(&dealer.hand));
                }
                fprintf(out, "DEALER STANDS - Dealer's final hand value: %d\n", getHandValue(&dealer.hand));
            }
            // case 2: S17, dealer stands on all 17s
            else
            {
                while (getHandValue(&dealer.hand) < 17) {
                addCardToHand(&dealer.hand, dealCard(&deck));
                fprintf(out, "DEALER HITTING - Dealer's NEW hand value: %d\n", getHandValue(&dealer.hand));
                }
                fprintf(out, "DEALER STANDS - Dealer's final hand value: %d\n", getHandValue(&dealer.hand));
            }
            
            // Determine winner, Player wins if dealer busts or has a higher hand
            if (isBust(&dealer.hand) || getHandValue(&player.hand) > getHandValue(&dealer.hand)) {
                fprintf(out, "Player wins!\n");
            }
            else if (getHandValue(&player.hand) < getHandValue(&dealer.hand)) {
                fprintf(out, "Dealer wins!\n");

            }
            else {
                fprintf(out, "It's a push!\n");
            }
            freeHand(&dealer.hand);
            freeHand(&player.hand);
        }
        freeDeck(&deck);
    }

    // Stop the clock, calculate the elapsed time, print the result
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Execution Time: %f seconds\n", cpu_time_used);

    return 0;

}

