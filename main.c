#include <stdio.h>
#include <stdlib.h>

#include <mysql/mysql.h>

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define STR_MAX 256

typedef struct {
    char itens[STR_MAX];
    size_t tamanho;
} string_t;

int main(void) {
    char *host = "localhost";
    char *user = "root";
    char *passwd = "root";
    char *db = "trab01";
    
    // Inicializar estrutura do banco de dados
    MYSQL *mysql = mysql_init(NULL);
    MYSQL_RES * res;
    MYSQL_ROW row;

    char lista_bancos[256];

    int qtde_bancos = 0;

    // Conectar com usuario mysql em um banco de dados
    if (!mysql_real_connect(mysql, host, user, passwd, db, 0, NULL, 0)) {
        fprintf(stderr, "ERRO: Impossivel conectar ao banco de dados.\n");
        fprintf(stderr, "%s\n", mysql_error(mysql));
        exit(1);
    }

    if (mysql_query(mysql, "call listar_bancos()")) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        exit(1);
    }

    res = mysql_use_result(mysql);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (qtde_bancos > 0) {
            strcat(lista_bancos, ";");
        }

        strcat(lista_bancos, row[0]);
        qtde_bancos += 1;
    }

    // Inicializa janela
    InitWindow(800, 450, "Information Schema Mysql");

    int bd_ativo = 0;
    bool db_aberto = false;

    // Loop de interface grafica
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            DrawText("Tabela:", 16, 16 + 64, GuiGetFont().baseSize, BLACK);
            if (GuiDropdownBox((Rectangle) {16, 20 + 16 + 64, 200, 32}, "Nada", &bd_ativo, db_aberto)) {
            }

            DrawText("Banco de Dados:", 16, 16, GuiGetFont().baseSize, BLACK);
            if (GuiDropdownBox((Rectangle) {16, 20 + 16, 200, 32}, lista_bancos, &bd_ativo, db_aberto)) {
                db_aberto = !db_aberto;
                if (!db_aberto) {
                    printf("selecionada opcao: %d\n", bd_ativo);
                }
            }
        EndDrawing();
    }

    // Fecha janela
    CloseWindow();

    // Encerra conexao com mysql
    mysql_free_result(res);
    mysql_close(mysql);

    return 0;
}
