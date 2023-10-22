CC = gcc
CFLAGS = -Wall -Wextra -std=c99
OBJS = linked_list.o markov_chain.o
LINKED_LIST_DEPS = linked_list.c linked_list.h
MARKOV_CHAIN_DEPS = markov_chain.c markov_chain.h
TWEETS_GENERATOR_DEPS = tweets_generator.c
SNAKES_AND_LADDERS_DEPS = snakes_and_ladders.c

linked_list.o: $(LINKED_LIST_DEPS)
	$(CC) $(CFLAGS) -c $<

markov_chain.o: $(MARKOV_CHAIN_DEPS)
	$(CC) $(CFLAGS) -c $<

tweets: $(OBJS) $(TWEETS_GENERATOR_DEPS)
	$(CC) $(CFLAGS) $(TWEETS_GENERATOR_DEPS) $(OBJS) -o tweets_generator

snake: $(OBJS) $(SNAKES_AND_LADDERS_DEPS)
	$(CC) $(CFLAGS) $(SNAKES_AND_LADDERS_DEPS) $(OBJS) -o snakes_and_ladders

clean:
	rm *.o snakes_and_ladders tweets_generator