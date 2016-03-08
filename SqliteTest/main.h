//
//  main.h
//  SqliteTest
//
//  Created by liuyu on 16/3/8.
//  Copyright © 2016年 liuyu. All rights reserved.
//

#ifndef main_h
#define main_h


#endif /* main_h */
#include <stdio.h>
#include "sqlite3.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>




const int recusion_max = 5;
const char* db_parent_path = "databases";
const char* p_path = "..";
const char* c_path = ".";
const int dir_type = 4;
const char* launcher_table_name = "favorites";
const int indicate_num = 3;
const char* indicate[] = {"screen","cellX","icon"};

void walk_dir(char *path, int depth);
char* cat_strings(char** str1, int count);
int is_right_struct(char* sql_string);
int change_data(sqlite3* db);
int exchange_if_needed(sqlite3* db, char* position);
int kill_appointed_process(char* name);
char *find_process(char* name);
char *trim(char * a);
char *del_enter(char *str);