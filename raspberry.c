#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <libpq-fe.h>

#define MAXZNAKU 13*19
#define MINZNAKU 5
#define ZNAKUNALINKU 19
#define POCETARDUIN 1

const char* lineout = "\e[1m#korzonarodni > \e[0m";

const unsigned int signals[] = {SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGIOT, SIGBUS, SIGFPE, SIGKILL, SIGUSR1,/* SIGSEGV,*/ SIGUSR2, SIGPIPE, SIGALRM, SIGTERM, SIGSTKFLT, SIGCHLD, SIGCONT, SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGURG, SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF, SIGWINCH, SIGIO, SIGPWR};

int fd;
int prevError = 0;
char *consoleLineBuff;
int deviceID = 0;
char psqlbuf[500];
PGresult *res;
PGconn *conn;

void sighandle(int signum){

}

void do_exit(PGconn *conn, PGresult *res){
	PQclear(res);
	PQfinish(conn);

	exit(1);
}

void linehandle(char *line) {
	memset(consoleLineBuff, 0, MAXZNAKU);

	if(strlen(line) >= MINZNAKU && strlen(line) <= MAXZNAKU){

		PQclear(res);

		snprintf(psqlbuf, 499, "INSERT INTO vzkazy (vzkaz,delka) values ('%s', %ld);", line, strlen(line));
		res = PQexec(conn, psqlbuf);

		if(PQresultStatus(res) != PGRES_COMMAND_OK){
			do_exit(conn, res);
		}

		prevError = 0;

		int untilEnd = ZNAKUNALINKU;

		serialPutchar(fd, deviceID+'a');

		printf("\nDekujeme za zpravu!\n\n", line);
		for(int i = 0; i < strlen(line); i++){
			if(*(line+i) == ' '){
				int wordLenght;
				for(wordLenght = 1; wordLenght < ZNAKUNALINKU; wordLenght++){
					if(*(line+i+wordLenght) == ' ' || *(line+i+wordLenght) == (char)0) break;
				}

				if(untilEnd < wordLenght){
					serialPutchar(fd,'\n');
					untilEnd = ZNAKUNALINKU;
					i++;
				}
			}
			delay(10);
			serialPutchar(fd, *(line+i));
			untilEnd--;
		}
		serialPutchar(fd, (char)0);
		deviceID++;
		if(deviceID >= POCETARDUIN){
			deviceID = 0;
		}

	}else if(strlen(line) < MINZNAKU){
		prevError = 1;
		memcpy(consoleLineBuff, line, sizeof(line));
		printf("Zprava ma jenom %d znaku :( zkuste nam jich napsat alespon %d\n", strlen(line) , MINZNAKU);
	}else if(strlen(line) > MAXZNAKU){
		prevError = 1;
		memcpy(consoleLineBuff, line, sizeof(line));
		printf("Zprava ma %d znaku, zkuste byt strucnejsi a zkratit zpravu na %d ;)\n", strlen(line), MAXZNAKU);
	}
}

int main(){
	consoleLineBuff = (char *) malloc(MAXZNAKU);
	conn = PQconnectdb("user=korzo password=zupLmPqZ dbname=vzkazy");

	if(PQstatus(conn) == CONNECTION_BAD){
		fprintf(stderr, "Nepodarilo se spojit s databazi: %s\n", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);

	}


	for(int i = 0; i < 28; i++){
		signal(signals[i], sighandle);
	}

	if ((fd = serialOpen("/dev/ttyS0",230400)) < 0) {
        	fprintf (stderr, "Nepodarilo se otevrit seriovy port: %s\n", strerror (errno)) ;
		free(consoleLineBuff);
        	return 1 ;
    	}

	if(wiringPiSetup () == -1) {
        	fprintf (stdout, "Nelze zapnout wiringPi: %s\n", strerror (errno)) ;
        	free(consoleLineBuff);
		return 1 ;
   	}

	rl_callback_handler_install(lineout, &linehandle);


	while(1){

		if(prevError){
			rl_insert_text(consoleLineBuff);
			rl_redisplay();
			prevError = 0;
		}

		rl_callback_read_char();
	}

	serialClose(fd);
	free(consoleLineBuff);
	PQclear(res);
	PQfinish(conn);
	return 0;

}
