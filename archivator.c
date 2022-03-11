#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

void structDir(char *dir, int depth);
int filewrite(char* filename, int out, bool isDir, int level, long size);

int main(int argc, char *argv[])			// ./archivator -d /home/... -o ./lab1.arch			путь к директории - входной аргумент -d флаг
{
	// обработка собственных аргументов

	char *sdir = argv[2];		//путь к директории
	printf("Directory : %s\n", sdir);
	//printdir("/home", 0);
	structDir(sdir, 0);
	printf("done.\n");
	exit(0);
}

void structDir(char *dir, int depth)		// проход по директории (определение структуры)
{
	DIR *dp;					//поток канала
	struct dirent *entry;		// элементы каталога возвращаются в структурах dirent 
	struct stat statbuf;		// данные о файле

	/* открывает каталог и формирует поток каталога. Возвращает указатель на структуру DIR, 
	которая будет использоваться для чтения элементов каталога*/
	if((dp = opendir(dir) == NULL)
	{
		//вывод в заданный файловый поток
		fprintf(stderr, "cannot open directory: %s\n", dir);
		return;
	}
	chdir(dir);		//перемешение в директорию
	while((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name, &statbuf);		//данные о ссылке?
		if(S_ISDIR(statduf.st_mode))		//проверка на каталог
		{
			//Находит каталог, но игнорирует . и .. 
			if(strcmp(".", entry->d_name) == 0 || strcmp("..", enty->d_name) == 0)
			continue;
			printf("%*s%s/\n", depth, "", entry->d_name);
			//Рекурсивный вызов с новый отступом 
			printdir(entry->d_name, depth+4);
		} else printf("%*s%s\n", depth, " ", entry->d_name);
	}
	chdir("..");	//возвращение вверх по дереву каталогов 
	closedir(dp);	// гарантирует, что кол-во открытых потоков каталогов не больше необходимого
}

// должно записаться последовательно, не затираться
int filewrite(char* filename, int out, bool isDir, int level, long size)
{
    printf("\n------------------------------------\n");
    printf("trying to write a file into archive\n");
    char buf[1024];
    int nread;
    int in = open(filename, O_RDONLY); // открытие только для чтения
	// in теперь дескриптор файла?

    if(in == -1)
        printf("failed to open a file!\n");
		// выход?

    char d = 'f';


    if(isDir)
        d = 'd';

	// out ~ fildes - файловый дескриптор 
    write(out, &d, 1); 				//запись одного байта 
    printf("type = %c\n", d);
    write(out, &level, sizeof(level));
    printf("level = %d\n", level);

    int namesize = strlen(filename);				// длина имени файла
    write(out, &namesize, sizeof(namesize)); 		//запись длины имени файла??
    write(out, filename, namesize);					// запись имени файла


    printf("filename = %s  size = %d\n", filename, namesize);
    write(out, &size, sizeof(size));				//запись размера файла
    printf("size of file = %ld\n", size);


    if(!isDir)
	{
		// запись содержимого файла (не директория)
        long sum = 0;
        while((nread = read(in, buf, sizeof(buf)) > 0)) // считывается, пока не конец файла
		{
            write(out, buf, nread);  //запись в аут nread байт, считанных в буфер
            sum += nread; //подсчет общего числа записанных байт
        }
        printf("successful writing of %ld bytes\n", sum);

    }
    printf("\n------------------------------------\n");
}