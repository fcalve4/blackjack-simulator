#ifndef HAND_H
#define HAND_H

#include "card.h"

typedef struct {
    Card *cards;  // Dynamically allocated array for cards
    int num_cards; // Number of cards in the hand
    int capacity; // Capacity of the hand

    // Booleans
    int soft_or_hard;
    int doubleable;
    int splittable;
    int surrenderable;
} Hand;

// Hand functions
void init_hand(Hand *hand);
void add_card_to_hand(Hand *hand, Card card);
int get_hand_value(const Hand *hand);
int is_bust(const Hand *hand);
void free_hand(Hand *hand);

int has_soft_ace(const Hand *hand);
int can_double(const Hand *hand);
int can_split(const Hand *hand);
int can_surrender(const Hand *hand);

#endif