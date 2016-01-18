//
//  main.c
//  SqliteTest
//
//  Created by liuyu on 15/12/4.
//  Copyright © 2015年 liuyu. All rights reserved.
//

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

void walk_dir(char *path, int depth, sqlite3* db);
char* combinallstring(char** str1, int count);
int is_right_struct(char* sql_string);

char** ignore_app_names;
char* position;
char* base_dir;
int main(int argc, const char* argv[]){
    char* errmsg = 0;
    int ret = 0;
    sqlite3 *db = 0;
    
//    if(argc <2)
//    {
//        printf("need base path arg");
//        return 1;
//    }
    base_dir = ".";
    if(argc == 2)
    {
        base_dir = argv[1];
    }else if(argc == 3)
    {
        base_dir = argv[1];
        position = argv[2];
    }else if(argc == 4)
    {
        base_dir = argv[1];
        position = argv[2];
        ignore_app_names = argv[3];
    }
    printf("main get argument:%s\n",argv[1]);
    walk_dir(base_dir, recusion_max, db);
    
    sqlite3_free(errmsg);
    sqlite3_close(db);
    return 0;
    
}

int change_data(sqlite3* db){
    printf("Begin change favorites table*************************************\n");
    char* error;
    if(position == NULL)
    {
        position = (char*)malloc(5);
        int max = get_max_screen(db);
        if(max == 0)
            sprintf(position, "%d", 3);
        //     		position = itoa(3);
        else
            sprintf(position, "%d", (max/2));
        //         	position = itoa(max/2);
    }
    int result = exchange_if_needed(db, position);
    printf("End change favorites table*************************************%d",result);
    return result;
}

//default (x,y) = (2,2)
int exchange_if_needed(sqlite3* db, char* position)
{
    //#Intent;action=android.intent.action.MAIN;category=android.intent.category.LAUNCHER;
    //component=sogou.mobile.explorer/.NoDisplayActivity;end
    char* update_sogou_temp[] =  {"update ",launcher_table_name, " set cellX = 2, cellY = 2, screen = ",position,
        " where intent like'%component=sogou.mobile.explorer/.NoDisplayActivity%'"};
    char** t_update_sogou = update_sogou_temp;
    const char* update_sogou_position_sql = combinallstring(t_update_sogou, 5);
    printf("update_sogou_position_sql : %s\n", update_sogou_position_sql);
    
    char* get_current_sogou_position_temp[] = {"select screen, cellX, cellY from ",launcher_table_name,
        " where intent like '%component=sogou.mobile.explorer/.NoDisplayActivity%'"};
    char** t_get_current_sogou_position = get_current_sogou_position_temp;
    const char* get_current_sogou_positon_sql = combinallstring(t_get_current_sogou_position, 3);
    printf("get_concurrent_sogou_positon_sql : %s\n", get_current_sogou_positon_sql);
    
    char** dbresult;
    int row,column;
    char* errormsg;
    char *sogou_screen, *sogou_cellX, *sogou_cellY;
    
    int result = sqlite3_get_table(db, get_current_sogou_positon_sql, &dbresult, &row, &column, &errormsg);
    if (result != SQLITE_OK)
    {
        printf("Read favorites table failed :%s \n", sqlite3_errmsg(db));
        return result;
    }
    else
    {
        int i = 0;
        for (i=column; i<(row+1)*column; i=i+column) {
            
            sogou_screen = dbresult[i];
            sogou_cellX = dbresult[i+1];
            sogou_cellY = dbresult[i+2];
        }
    }
    char* get_specify_position_temp[] = {"select _id from ",launcher_table_name, " where cellX = 2 and cellY = 2 and screen = ", position};
    char** t_get_specify_position = get_specify_position_temp;
    const char* get_specify_position_sql = combinallstring(t_get_specify_position, 4);
    printf("get_specify_position_sql : %s\n", get_specify_position_sql);
    
    char *_id;
    
    result = sqlite3_get_table(db, get_specify_position_sql, &dbresult, &row, &column, &errormsg);
    if(result != SQLITE_OK)
    {
        int ret = sqlite3_exec(db, update_sogou_position_sql, NULL, NULL, &errormsg);
        return ret;
    }
    else
    {
        int ret1,ret2;
        if (row != 0) {
            int i = 0;
            for (i = column; i<(row+1)*column; i = i+column) {
                _id = dbresult[i];
            }
            char* update_specify_position[] = {"update ",launcher_table_name, " set screen = ", sogou_screen, ", cellX = ", sogou_cellX,
                ", cellY = ", sogou_cellY, " where _id = ", _id};
            char** t_update_specify_position = update_specify_position;
            const char* update_specify_position_sql = combinallstring(t_update_specify_position, 10);
            printf("update_specify_position_sql : %s\n", update_specify_position_sql);
            
            ret1 = sqlite3_exec(db, update_specify_position_sql, NULL, NULL, &errormsg);

        }
        
        ret2 = sqlite3_exec(db, update_sogou_position_sql, NULL, NULL, &errormsg);
        return ret1|ret2;
    }
}
int get_max_screen(sqlite3* db){
    
    const char* get_max_sql = "select max(screen) from favorites";
    char** dbresult;
    int row,column, max;
    char* errormsg;
    
    int result = sqlite3_get_table(db, get_max_sql, &dbresult, &row, &column, &errormsg);
    if (result != SQLITE_OK)
    {
        printf("Read favorites table failed :%s \n", sqlite3_errmsg(db));
        return 0;
    }
    else
    {
        int i = 0;
        for (i=column; i<(row+1)*column; i=i+column) {
            
            max = atoi(dbresult[i]);
        }
    }
    return max;
}

int try_modify_favorites_data(const char* dbname, sqlite3* db)
{
    int result = sqlite3_open(dbname, &db);
    if(result != SQLITE_OK)
    {
        printf("\n Cannot open db, error : %s\n",sqlite3_errmsg(db));
        return result;
    }
    
    const char* sql_select_all_tables = "select name, sql from sqlite_master";
    
    char** dbresult;
    int row,column,index;
    char* errormsg;
    int result1 = sqlite3_get_table(db, sql_select_all_tables, &dbresult, &row, &column, &errormsg);
    
    if (result1 != SQLITE_OK)
    {
        printf("Read sqlite_mater table failed :%s \n", sqlite3_errmsg(db));
        return result1;
    }
    else
    {
        
        index = row;
        char* table_name = 0;
        char* table_sql = 0;
        int i = 0;
        for (i=column; i<(row+1)*column; i=i+column) {
            
            table_name = dbresult[i];
            table_sql = dbresult[i+1];
            if (table_sql == NULL) {
                continue;
            }
            printf("\n +++++++++Read table_name:%s:::%d\n",table_name,is_right_struct(table_sql));
            if ((strcasecmp(launcher_table_name, table_name) == 0) && (is_right_struct(table_sql))) {
                printf("\n find Right Table Name : %s\n", table_name);
                change_data(db);
            }
        }
        
    }
    
    return 0;
}

int is_right_struct(char* sql_string)
{
    char *sql_string1 = "CREATE TABLE favorites (_id INTEGER PRIMARY KEY,title TEXT,intent TEXT,container INTEGER,screen INTEGER,cellX INTEGER,cellY INTEGER,spanX INTEGER,spanY INTEGER,itemType INTEGER,appWidgetId INTEGER NOT NULL DEFAULT -1,isShortcut INTEGER,iconType INTEGER,iconPackage TEXT,iconResource TEXT,icon BLOB,uri TEXT,displayMode INTEGER)";
    int i = strstr(sql_string1,indicate[0]);
    int j = strstr(sql_string1,indicate[1]);
    int k = strstr(sql_string1,indicate[2]);
    printf("\n sql:%s::%d::%d::%d",sql_string,i,j,k);
    if (i > 0 && j > 0 && k > 0)
        return 1;
    else
        return 0;
}

void walk_dir(char *path, int depth, sqlite3* db)
{
    if (depth<=0) {
        return;
    }
    
    char* current_path = path;
    printf("%d:路径为[%s]\n", depth,path);
    
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(path);
    
    
    while (NULL != pDir && NULL != (ent=readdir(pDir)))
    {
        char* file_name = ent->d_name;
        if (!strcasecmp(file_name, p_path) || !strcasecmp(file_name, c_path)) {
            continue;
        }
        int file_type = ent->d_type;
        char* a[] = {current_path, "/", file_name};
        char** b = a;
        char* real_path = combinallstring(b, 3);
        
        
        if (file_type == dir_type) {
            
            walk_dir(real_path, depth-1, db);
        }
        else
        {
            char *s = strstr(file_name, ".db");
            if(NULL != s && strcasecmp(s,".db") == 0)
            {
                try_modify_favorites_data(real_path, db);
            }
        }
        
    }
}

char* combinallstring(char** str1, int count)
{
    char* last = 0;
    int totallen = 0;
    int i = 0;
    last = (char*)malloc(sizeof(char)*(2048));
    memset(last, 0, 2048);
    for (i=0; i<count; i++) {
        
        memcpy(last+totallen, str1[i], strlen(str1[i]));
        totallen += strlen(str1[i]);
    } 
    return last;
}
