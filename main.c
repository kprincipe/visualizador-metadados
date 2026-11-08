#include <stdio.h>
#include <stdlib.h>

#include <mysql/mysql.h>

#include "raylib.h"

// TODO: Achar uma maneira de nao precisar alocar esse espaco estaticamente
#define RAYGUI_TEXTSPLIT_MAX_ITEMS          128 * 4
#define RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE     1024 * 4

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define UTILS_IMPLEMENTATION
#include "utils.h"

#define STR_MAX 1024 * 8

typedef struct {
    Rectangle posicao;
} combobox_estado;

typedef enum {
    PRINCIPAL = 0,
    REGISTRO
} estado_t;

String_View extract_item(String_Builder *sb, int i) {
    int pos = 0, indice = 0;
    int max = 0;

    while (pos < sb->count) {
        if (sb->items[pos++] == ';') max++;
    }

    pos = 0;

    String_View res = {0};

    int item_sz = 0;
    while (pos < sb->count && indice <= max) {
        if (sb->items[pos] == ';') {
            if (indice == i) break;
            indice++;
            item_sz = 0;
        }
        item_sz++;
        pos++;
    }

    res.items = (sb->items + pos) - item_sz;
    res.count = item_sz;

    if (*res.items == ';') {
        res.items++;
        res.count--;
    }

    return res;
}

int main(void) {
    char *host = "localhost";
    char *user = "root";
    char *passwd = "root";
    char *db = "trab01";
    
    // Inicializar estrutura do banco de dados
    MYSQL *mysql = mysql_init(NULL);
    MYSQL_RES * res;
    MYSQL_ROW row;

    char query[STR_MAX];

    String_Builder bancos = {0};
    String_Builder tabelas = {0};
    String_Builder atributos = {0};

    // Conectar com usuario mysql em um banco de dados
    if (!mysql_real_connect(mysql, host, user, passwd, db, 0, NULL, 0)) {
        fprintf(stderr, "ERRO: Impossivel conectar ao banco de dados.\n");
        fprintf(stderr, "%s\n", mysql_error(mysql));
        exit(1);
    }

    sprintf(query, "call listar_bancos()");
    if (mysql_query(mysql, query)) {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        exit(1);
    }

    res = mysql_use_result(mysql);

    while ((row = mysql_fetch_row(res)) != NULL) {
        if (bancos.count != 0) da_append(&bancos, ';');
        util_sb_from_cstr(&bancos, row[0]);
    }
    da_append(&bancos, '\0');

    mysql_free_result(res);

    // Inicializa janela
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Information Schema Mysql");

    int bd_selecionado = 0;
    int tb_ativo = 0;

    int listViewScrollIndex = 0;
    int listViewActive = 0;

    bool db_aberto = false, tb_aberto = false;

    combobox_estado tabelas_estado = {
        .posicao = { 16, 20 + 16 + 64, (GetScreenWidth() / 3), GetScreenHeight() - 128 }
    };

    combobox_estado bancos_estado = {
        .posicao = { 16, 20 + 16, GetScreenWidth() / 3, 32 }
    };

    Font font = LoadFontEx("./fonte/IosevkaNerdFont-Regular.ttf", 16, NULL, 100);
    GuiSetFont(font);

    // Loop de interface grafica
    while (!WindowShouldClose()) {
        BeginDrawing();
            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize);

            ClearBackground(RAYWHITE);

            GuiDisable();
            GuiButton((Rectangle) { bancos_estado.posicao.x + 312, bancos_estado.posicao.y, 200, 32 }, "Inserir dado");
            GuiEnable();

            GuiDrawText("Tabela:", (Rectangle) { 16, 12 + 64, 200, 32 }, 0, BLACK);
        
            tabelas_estado.posicao.height = GetScreenHeight() - 128;
            
            if (db_aberto) GuiDisable();
            if (GuiListView(tabelas_estado.posicao, tabelas.items, &listViewScrollIndex, &listViewActive));
            GuiEnable();

            GuiDrawText("Banco de Dados:", (Rectangle) { 16, 12, 200, 32 }, 0, BLACK);
            if (GuiDropdownBox(bancos_estado.posicao, bancos.items, &bd_selecionado, db_aberto)) {
                db_aberto = !db_aberto;
                if (!db_aberto) {
                    *query = '\0';
                    String_View bd = extract_item(&bancos, bd_selecionado);
                    sprintf(query, "call listar_tabelas('%.*s')", bd.count, bd.items);

                    mysql_next_result(mysql);
                    if (mysql_query(mysql, query)) {
                        fprintf(stderr, "%s\n", mysql_error(mysql));
                        exit(1);
                    }
                        
                    res = mysql_use_result(mysql);
                    
                    tabelas.count = 0;
                    while ((row = mysql_fetch_row(res)) != NULL) {
                        if (tabelas.count != 0) da_append(&tabelas, ';');
                        util_sb_from_cstr(&tabelas, row[0]);
                    }
                    da_append(&tabelas, '\0');

                    mysql_free_result(res);
                }
            }
        EndDrawing();
    }

    // Fecha janela
    CloseWindow();

    // Encerra conexao com mysql
    mysql_close(mysql);

    return 0;
}
