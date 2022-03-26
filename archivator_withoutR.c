#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pwd.h>

void structDir(char *dir, int out, int depth);
int filewrite(char* filename, int out, bool isDir, int depth, long size);

int fileread_fstep(char* filename, char *sdir);
void fileread(int in, int depth);
int createF(int in);
DIR* createD(char* filename, int depth);

int main(int argc, char *argv[])			// ./archivator -d /home/... -o ./lab1.arch			путь к директории - входной аргумент -d флаг
{											// ./archivator -f ./lab1.arch -d /home/...	
	// обработка аргументов
	//printf(" argv[1] = %s\n", argv[1]); 
	if (argv[1][1] == 'd')
	{
			char *sdir = argv[2];		//путь к директории
			char *outf = argv[4];
			printf("Directory : %s\n", sdir);
			//printdir("/home", 0);
			int out = open(outf, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
			structDir(sdir, out, 0);
			close(out);
	
	}	if (argv[1][1] == 'f')
		{
			char *sdir = argv[4];		
			char *inf = argv[2];
			printf("File : %s\n", inf);
			//printdir("/home", 0);
			fileread_fstep(inf, sdir);

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
	
	if((dp = opendir(dir)) == NULL)
	{
		//вывод в заданный файловый поток
		fprintf(stderr, "cannot open directory: %s\n", dir);
		return;
	}
	printf("trying to open dir\n");
	chdir(dir);		//перемешение в директорию
	while((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name, &statbuf);		//данные о ссылке?
		if(S_ISDIR(statbuf.st_mode))		//проверка на каталог
		{
			//Находит каталог, но игнорирует . и .. 
			if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
			continue;
			isDir = true;
			///printf("%*s%s/\n", depth, "", entry->d_name);
			filewrite(entry->d_name, out, isDir, depth, statbuf.st_size);
			//Рекурсивный вызов 
			structDir(entry->d_name, out, depth + 1);
		} else filewrite(entry->d_name, out, false, depth, statbuf.st_size);	// запись файла
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
	// in теперь дескриптор файла

    if(in == -1)
        printf("failed to open a file!\n");
		// выход?

    char type = 'f';


    if(isDir)
        type = 'd';

	// out ~ fildes - файловый дескриптор 
	write(out, &depth, sizeof(depth));
    printf("depth = %d\n", depth);

    write(out, &type, 1); 				//запись одного байта 
    printf("type = %c\n", type);

    int namesize = strlen(filename);				// длина имени файла
    write(out, &namesize, sizeof(namesize)); 		//запись длины имени файла??
    write(out, filename, namesize + 1);					// запись имени файла

		//lstat(filename, &statbuf);		//Время последнего изменения 
		//write(out, statbuf.st_mtime, sizeof(statbuf.st_mtime));

    printf("filename = %s  size of name = %d\n", filename, namesize);
    write(out, &size, sizeof(size));				//запись размера файла
    printf("size of file = %ld\n", size);


    if(!isDir)
	{
		// запись содержимого файла (не директория)
        long sum = 0;
        while((nread = read(in, buf, sizeof(buf))) > 0) // считывается, пока не конец файла
		{
            write(out, buf, nread);  //запись в аут nread байт, считанных в буфер
            sum += nread; //подсчет общего числа записанных байт
        }
        printf("successful writing of %ld bytes\n", sum);

    }
	close(in);
    printf("\n------------------------------------\n");
}
int fileread_fstep(char* filename, char *sdir)
{
	DIR* dp;
	printf("trying to open input file\n");
	int in = open(filename, O_RDONLY); // открытие только для чтения
	if(in == -1)
    printf("failed to open a input file!\n");
 	chdir(sdir);
	//dp = createD("arch", -1);
	if(mkdir("arch", S_IRWXU|S_IRWXG|S_IRWXO) == -1)		// create dir
			fprintf(stderr, "cannot create directory: %s\n", "arch");
	chdir("arch");
	fileread(in, 0);
	close(in);
	//closedir(dp);

}
void fileread(int in, int depth)
{
	
    char buf[1024];
	//bool ret = false;
	char type;
	char* outfile;
	DIR* dp;
    int dir, dep = 0, nread, namesize;
	long int size;
	 

 while (true) 
 {
	//if (!ret) 
	
		nread = read(in, &dep, sizeof(int));
		if(nread == 0) return;	// считывается, пока не конец файла
	
	if (dep < depth) 
	{
		fprintf(stderr, "current depth = %d filedep = %d\n", depth, dep);
			dir = chdir("..");	//возвращение вверх по дереву каталогов
			getcwd(buf, sizeof(buf));
		//if (buf != filename)
		fprintf(stderr, "Chdir return %d! Current directory: %s\n", dir, buf); 
		//closedir(dp);	// гарантирует, что кол-во открытых потоков каталогов не больше необходимого
		depth--;
	}
	nread = read(in, &type, 1);
	//if(nread == 0) return;	// считывается, пока не конец файла
	if(nread == -1) 
	printf("failed to read a file!\n");
	
	printf("type = %c\n", type);
	if(type == 'f')
	{
		createF(in);
		//ret = false;
		//fileread(outfile, in, depth, 'f');
	} 

	if (type == 'd')
	{
		
		read(in, &namesize, sizeof(int));		// считан размер имени файла
		char* outd = (char*)malloc(namesize+1);
		read(in, outd, namesize+1);			// считано имя файла
		
		read(in, &size, sizeof(long int));
		// createDir
		
	
		if(mkdir(outd, S_IRWXU|S_IRWXG|S_IRWXO) == -1)		// create dir
			fprintf(stderr, "cannot create directory: %s\n", outd);
				
		/*if((dp = opendir(outd)) == NULL)
		{
			//вывод в заданный файловый поток
			fprintf(stderr, "cannot open directory: %s\n", outd);
			//return dp;
		} */
		dir = chdir(outd);
		getcwd(buf, sizeof(buf));
		//if (buf != filename)
		fprintf(stderr, "Chdir return %d! Current directory: %s\n", dir, buf);
		depth++;
		//ret = true;
		free(outd);
	}
 }

	
}
int createF(int in)
{
	//char* outfile;
	char buf[1024];
	int nread, namesize, dir;
	long int size, sum = 0;
	printf("trying to read info!\n");
		//read(in, &depth, sizeof(depth));
		//printf("depth = %d\n", depth);
		read(in, &namesize, sizeof(namesize));		// считан размер имени файла
		printf("namesize = %d\n", namesize);

		char* outfile = (char*)malloc(namesize+1);
		read(in, outfile, namesize+1);			// считано имя файла
		printf("filename= %s\n", outfile);
		read(in, &size, sizeof(long int));
		printf("size = %ld\n", size);

		printf("trying to open a file %s!\n", outfile);
		int out = open(outfile, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
		if(out == -1)
		{
			printf("failed to open a file!\n");
			//exit(-3);
			return -3;
		}
        	
		
		while(sum != size) 
		{
			if((size - sum) < sizeof(buf))
			{
				nread = read(in, buf, size - sum);
            	
			} else nread = read(in, buf, sizeof(buf));
            		
			write(out, buf, nread);  //запись в аут nread байт, считанных в буфер
            sum += nread; //подсчет общего числа записанных байт
        }
        printf("successful writing of %ld bytes\n", sum);
		free(outfile);
		close(out);
}
DIR* createD(char* filename, int depth)
{
	char buf[1024];
	int dir;
		DIR *dp;
	
		if(mkdir(filename, S_IRWXU|S_IRWXG|S_IRWXO) == -1)		// create dir
			fprintf(stderr, "cannot create directory: %s\n", filename);
				
		if((dp = opendir(filename)) == NULL)
		{
			//вывод в заданный файловый поток
			fprintf(stderr, "cannot open directory: %s\n", filename);
			//return dp;
		}
		dir = chdir(filename);
		getcwd(buf, sizeof(buf));
		//if (buf != filename)
		fprintf(stderr, "Chdir return %d! Current directory: %s\n", dir, buf);
		depth++;
	return dp;
}
