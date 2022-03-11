#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

void structDir(char *dir, int out, int depth);
int filewrite(char* filename, int out, bool isDir, int depth, long size);
int fileread(char* filename);

int main(int argc, char *argv[])			// ./archivator -d /home/... -o ./lab1.arch			путь к директории - входной аргумент -d флаг
{											// ./archivator -f ./lab1.arch -d /home/...	
	// обработка аргументов
	if (argv[1] = "-d")
	{
			char *sdir = argv[2];		//путь к директории
			char *outf = argv[4];
			printf("Directory : %s\n", sdir);
			//printdir("/home", 0);
			int out = open(outf, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
			structDir(sdir, out, 0);
	
	}	if (argv[1] = "-f")
		{
			char *sdir = argv[4];		
			char *inf = argv[2];
			printf("File : %s\n", inf);
			//printdir("/home", 0);
			fileread(inf);

		}	

	printf("done.\n");
	exit(0);
}

void structDir(char *dir, int out, int depth)		// проход по директории (определение структуры)
{
	DIR *dp;					//поток канала
	struct dirent *entry;		// элементы каталога возвращаются в структурах dirent 
	struct stat statbuf;		// данные о файле
	bool isDir = false;

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
		if(S_ISDIR(statbuf.st_mode))		//проверка на каталог
		{
			//Находит каталог, но игнорирует . и .. 
			if(strcmp(".", entry->d_name) == 0 || strcmp("..", enty->d_name) == 0)
			continue;
			isDir = true;
			///printf("%*s%s/\n", depth, "", entry->d_name);
			filewrite(entry->d_name, out, isDir, depth, sizeof(entry))
			//Рекурсивный вызов 
			structDir(entry->d_name, out, depth + 1);
		} else filewrite(entry->d_name, out, isDir, depth, sizeof(entry))	// запись файла
	}
	chdir("..");	//возвращение вверх по дереву каталогов 
	closedir(dp);	// гарантирует, что кол-во открытых потоков каталогов не больше необходимого
}

// должно записаться последовательно, не затираться
int filewrite(char* filename, int out, bool isDir, int depth, long size)
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

    char type = 'f';


    if(isDir)
        type = 'd';

	// out ~ fildes - файловый дескриптор 
    write(out, &type, 1); 				//запись одного байта 
    printf("type = %c\n", type);
    write(out, &depth, sizeof(depth));
    printf("depth = %d\n", depth);

    int namesize = strlen(filename);				// длина имени файла
    write(out, &namesize, sizeof(namesize)); 		//запись длины имени файла??
    write(out, filename, namesize);					// запись имени файла

		//lstat(filename, &statbuf);		//Время последнего изменения 
		//write(out, statbuf.st_mtime, sizeof(statbuf.st_mtime));

    printf("filename = %s  size of name = %d\n", filename, namesize);
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
	close(in);
    printf("\n------------------------------------\n");
}
int fileread(char* filename)
{
	printf("trying to read a file from archive\n");
    char buf[1024];
	char type;
	char* outfile;
    int nread, depth, namesize, dir;
	long int size, sum;
    int in = open(filename, O_RDONLY); // открытие только для чтения
	    if(in == -1)
        printf("failed to open a file!\n");

	dir = mkdir(filename); 				// create dir
	if (nread = read(in, &type, 1) != 1)
	printf("failed to read a file!\n");

	if(type == 'f')
	{
		read(in, &depth, sizeof(int));
		read(in, &namesize, sizeof(int));		// считан размер имени файла
		read(in, outfile, namesize);			// считано имя файла
		read(in, &size, sizeof(long int));

		int out = open(outfile, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
		if(out == -1)
        printf("failed to open a file!\n");

		while(sum != size) 
		{
			nread = read(in, buf, sizeof(buf));
            write(out, buf, nread);  //запись в аут nread байт, считанных в буфер
            sum += nread; //подсчет общего числа записанных байт
        }
        printf("successful writing of %ld bytes\n", sum);
		close(out);
	}

}
