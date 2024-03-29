/**
 ** @file interpreter.c
 ** @author Cesar Alejandro Lara Limon. larachicharo@protonmail.com
 ** @author Alejandro Tonatiuh Valderrama Silva valdefrog@gmail.com
 ** @brief little sqlite3 example
 **
 **/
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

//the extention we accept
#define EXTENTION "sql"
#define DBNAME "DataBase.db"

sqlite3 *ppDb;

void write_db (char* file, int rc);
int print_query_callback(
	void *data, int argc, char **argv, char **azColName);
void handle_rc(int rc, sqlite3 **ppDb);
int query_instagram_by_id_callback(
	void *data, int argc, char **argv, char **azColName);
int query_instagramid_by_morraid_callback(
	void *data, int argc, char **argv, char **azColName);
int query_morras_by_country_callback(
	void *data, int argc, char **argv, char **azColName);
void query_russian_instagram(void);
void print_menu(void);
void eliminate_repeated_file();
int file_exist ();

/**
 * Reads a .sql file, loads its instructions to a database
 * and then provides a method to explore the contents of 
 * the newly created database.
 * @param argc the number of arguments received
 * @param argv the arguments received.
 * @return, returns 0 if everything went successfull.
 *					1 otherwise
 */
int main(int argc, char* argv[]){
	//Because of a good practice, we check the arguments
	if(argc == 1){
		printf("There is not a file as an argument\n");
		exit(1);
	}
	char *ext = strrchr(argv[1], '.');
	if(strcmp(ext+1, EXTENTION) != 0){
		printf("The extension of the file isn't valid\n");
		exit(1);
	}	
	//It's necessary to remove an existing database with the same name.
	//Otherwise the program won't work
	eliminate_repeated_file(argv[1]);
	int rc = sqlite3_open(DBNAME, &ppDb);
	write_db(argv[1], rc);
	char *errmsg;

	handle_rc(rc, &ppDb);

	char c;
	char *q;
	print_menu();
	while((c = getchar()) != 'q') {
		if (c == '\n')
			continue;
		if (c == 'a')
			q = "SELECT * FROM instagram ORDER BY nfollowers DESC LIMIT 5;";
			rc = sqlite3_exec(ppDb, q, print_query_callback, NULL, &errmsg);
			handle_rc(rc, &ppDb);
		if (c == 'r')
			query_russian_instagram();		
		
		print_menu();
	}
	sqlite3_close(ppDb);

	return 0;
}

/**
 * This method writes the informatiion into de database.
 * @param file, the name of the file with the information of the databse
 */
void write_db (char* file, int rc){
	int i, j;
	char *err_msg = 0;
	char block[1024];
	char sql[100000];
	FILE *read = fopen(file, "r");

	if(rc){
    	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(ppDb));
    	//finish in case of an error
    	exit(1);
   	}
   	else
      	fprintf(stderr, "Opened database successfully\n");

    //here we read the file  
    if(file_exist(file) == 1){
    	printf("File provied doesn't exist\n");
    	exit(1);
    }
	for (i = getc(read); i != EOF; i = getc(read))
		sql[j++] = i;

	//adding data to database
	rc = sqlite3_exec(ppDb, sql, 0, 0, &err_msg);

	if (rc != SQLITE_OK){
        fprintf(stderr, "SQLITE error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(ppDb);
       	//finish in case of an error
        exit(1);
    }
}

/**
 * Callback function invoked by sqlite3.
 * Prints a set of data, the result of the query.
 * @param data argument sent by user.
 * @param argc the number of arguments in the param argv
 * @param argv array of results of query
 * @param argv array of names of columns
 * @returns status code.
 */
int print_query_callback(
	void *data, int argc, char **argv, char **azColName) {
	
	int i;
	for (i = 0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}   
	printf("\n");
	return 0;
}

/**
 * Handles errors reported/produced by sqlite3
 * @param rc status code
 * @param ppDb pointer to pointer of database instance
 */
void handle_rc(int rc, sqlite3 **ppDb) {
	if (rc != SQLITE_OK) {
		fprintf(stderr, "%s\n",sqlite3_errmsg(*ppDb));
		sqlite3_close(*ppDb);
		exit(1);
	}
}

/**
 * Callback function invoked by sqlite3.
 * Fetches instagrams by its id.
 * @param data argument sent by user.
 * @param argc the number of arguments in the param argv
 * @param argv array of results of query
 * @param argv array of names of columns
 * @returns status code.
 */
int query_instagram_by_id_callback(
	void *data, int argc, char **argv, char **azColName) {
	if (argc < 1)
		return 1;
	int rc;
	char *q;
	char *q_template;
	char *errmsg;
	q_template = "SELECT * FROM instagram WHERE id=%s;";
	sprintf(q, q_template, argv[0]);
	rc = sqlite3_exec(
		ppDb, q, print_query_callback, NULL, &errmsg);
	handle_rc(rc, &ppDb);
	return 0;

}

/**
 * Callback function invoked by sqlite3. 
 * Fetches id's of instagrams that belong to certain morras.
 * The specific morras are specified in argv
 * @param data argument sent by user.
 * @param argc the number of arguments in the param argv
 * @param argv array of results of query
 * @param argv array of names of columns
 * @returns status code.
 */
int query_instagramid_by_morraid_callback(
	void *data, int argc, char **argv, char **azColName) {

	if (argc < 1)
		return 1;
	int rc;
	char *q;
	char *q_template;
	char *errmsg;
	q_template = "SELECT instagram_id FROM morra_instagram WHERE morra_id=%s;";
	sprintf(q, q_template, argv[0]);
	rc = sqlite3_exec(
		ppDb, q, query_instagram_by_id_callback, NULL, &errmsg);
	handle_rc(rc, &ppDb);
	return 0;
}

/**
 * Callback function. Receives an id of a country and 
 * gets all the morras who live there.
 * @param data argument sent by user.
 * @param argc the number of arguments in the param argv
 * @param argv array of results of query
 * @param argv array of names of columns
 * @returns status code.
 */
int query_morras_by_country_callback(
	void *data, int argc, char **argv, char **azColName) {
	
	if (argc < 1)
		return 1;
	int rc;
	char *q;
	char *q_template;
	char *errmsg;
	q_template = "SELECT id FROM morra WHERE country_id=%s;";
	sprintf(q, q_template, argv[0]);
	rc = sqlite3_exec(
		ppDb, q, query_instagramid_by_morraid_callback, NULL, &errmsg);
	handle_rc(rc, &ppDb);
	return 0;
} 

/**
 * Queries all the instagrams that belong to Russian girls.
 */
void query_russian_instagram(void) {
	char *errmsg;
	char *q;
	q = "SELECT id FROM country WHERE name='Rusia';";
	int rc;
	rc = sqlite3_exec(
		ppDb, q, query_morras_by_country_callback, NULL, &errmsg);
	handle_rc(rc, &ppDb);
}

/**
 * Creates a menu, with to options, for the user.
 * With the option 'a', the user will be able to see the 5 most followed instagrams
 * and with 'r' the russian instagram accounts.
 */
void print_menu(void) {
	puts("Press: q to quit");
	puts("a to show the 5 most followed instagram accounts");
	puts("r to show russian insagram accounts.");
}

/**
 * Eliminates the file, if it exists
 */
void eliminate_repeated_file(){
	if(file_exist(DBNAME) == 0)
		remove(DBNAME);
}


/**
 * Veryfies if the database already exists
 * @return 0 if the file exists
 *		   1 otherwise
 */
int file_exist (char* filename){
	FILE* file;
    if((file = fopen(filename,"r"))!=NULL){
            fclose(file);
            return 0;
        }
    return 1;
}
