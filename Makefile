all:
	gcc -o archivator -fsanitize=address ./archivator.c
	
