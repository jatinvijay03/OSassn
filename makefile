all: client ps ss cleanup lb

client: client.c
	gcc -o client client.c

ps: primary_server.c
	gcc -o ps primary_server.c

ss: secondary_server.c
	gcc -o ss secondary_server.c

lb: load_balancer.c
	gcc -o lb load_balancer.c

cleanup: cleanup.c
	gcc -o cleanup cleanup.c