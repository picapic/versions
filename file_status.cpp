#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/select.h>

int gIsChanged;

//обработчик сигнала
void sig_handler(int nsig, siginfo_t *siginfo, void *context)
{
    if(nsig == SIGIO)
    {
	//fprintf(stderr, "Some operation with file\n");
	gIsChanged = 1;
    }
}

int main(int argc, char *argv[])
{
    int fd;
    char filename[256];
    char buff[2048];
    struct stat st;
    sigset_t set;
    struct sigaction signal_data;

    //проверяем количество аргументов
    if(argc <= 1)
    {
	fprintf(stderr, "USAGE: file_status <filename>\n");
	return -1;
    }

    //обнуляем имя файла
    memset(filename, 0, sizeof(filename));
    //копируем имя файла
    strncpy(filename, argv[1], sizeof(filename));
    
    //пытаемся открыть файл
    fd = open(filename, O_RDONLY);
    //при ошибке - выходим
    if(fd < 0)
    {
	fprintf(stderr, "Can not open file %s\n", filename);
	return -2;
    }

    //обнуляем описание сигнала
    memset(&signal_data, 0, sizeof(signal_data));
    //назначаем обработчик сигнала
    signal_data.sa_sigaction = &sig_handler;
    //сопровождаем сигнал дополнительной информацией
    signal_data.sa_flags = SA_SIGINFO;
    //обнуляем маску блокируемых сигналов
    sigemptyset(&set);
    //инициализируем маску
    signal_data.sa_mask = set;
    //подключаем сигнал
    if(sigaction(SIGIO, &signal_data, NULL) < 0)
    {
	close(fd);
	fprintf(stderr, "Can not activate signal\n");
	return -3;
    }

    //вешаем сигнал на дескриптор
    if(fcntl(fd, F_SETSIG, 0) < 0)
    {
	close(fd);
	fprintf(stderr, "Can not init signal for fd\n");
	return -4;
    }
    //устанавливаем типы оповещений
    if(fcntl(fd, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_RENAME) < 0)
    {
	close(fd);
	fprintf(stderr, "Can not set types for the signal\n");
	return -5;
    }

    for(;;)
    {
	usleep(100000);
	if(gIsChanged == 1)
	{
	    //вешаем сигнал на дескриптор
	    if(fcntl(fd, F_SETSIG, 0) < 0)
	    {
		close(fd);
		fprintf(stderr, "Can not init signal for fd\n");
		return -6;
	    }
	    //устанавливаем типы оповещений
	    if(fcntl(fd, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_RENAME) < 0)
	    {
		close(fd);
		fprintf(stderr, "Can not set types for the signal\n");
		return -7;
	    }
	    fprintf(stderr, "Some operation with file\n");
	    gIsChanged = 0;
	}
    }
    
    //получаем данные а файле
    fstat(fd, &st);

    //обнууляем буфер
    memset(buff, 0, sizeof(buff));
    //считываем файл в буфер
    read(fd, buff, st.st_size);
    
    //выводим содуржимое буфера
    fprintf(stderr, "%s\n", buff);
    
    //закрываем файл
    close(fd);
    
    return 0;
}
