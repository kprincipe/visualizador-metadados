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
    String_View banco_selecionado = {0};

    String_Builder tabelas = {0};
    String_View tabela_selecionada = {0};

    String_Builder atributos = {0};
    String_View atributo_selecionado = {0};

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
    InitWindow(920, 720, "Information Schema Mysql");

    int bd_indice = 0;
    int tb_ativo = 0;

    int listViewScrollIndex = 0;
    int tabela_indice = 0;

    bool db_aberto = false, tb_aberto = false, reg_aberto = false;

    bool modo_editar[5] = {false};

    Font font = LoadFontEx("./fonte/IosevkaNerdFont-Regular.ttf", 16, NULL, 100);
    GuiSetFont(font);

    // Loop de interface grafica
    while (!WindowShouldClose()) {
        BeginDrawing();
            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize);

            ClearBackground(RAYWHITE);
            
            char tmp_str[STR_MAX];
            sprintf(tmp_str, "%.*s",
                    tabela_selecionada.count,
                    tabela_selecionada.items);

            if (strcmp(tmp_str, "venda") != 0) GuiDisable();

            if (GuiButton((Rectangle) { 16 + 312, 20 + 16, 200, 32 }, "Inserir dado")) {
                reg_aberto = true;
            }

            if (!reg_aberto) GuiEnable();

            GuiDrawText("Tabela:", (Rectangle) { 16, 12 + 64, 200, 32 }, 0, BLACK);
            if (db_aberto) GuiDisable();

            if (GuiListView((Rectangle) { 16, 20 + 16 + 64, (GetScreenWidth() / 3), GetScreenHeight() - 128 }, tabelas.items, &listViewScrollIndex, &tabela_indice)) {
                tabela_selecionada = extract_item(&tabelas, tabela_indice);
                
                *query = '\0';
                sprintf(query, "call listar_atributos('%.*s', '%.*s')",
                        banco_selecionado.count, banco_selecionado.items,
                        tabela_selecionada.count, tabela_selecionada.items);

                mysql_next_result(mysql);
                if (mysql_query(mysql, query)) {
                    fprintf(stderr, "%s\n", mysql_error(mysql));
                    exit(1);
                }

                res = mysql_use_result(mysql);

                atributos.count = 0;
                while ((row = mysql_fetch_row(res)) != NULL) {
                    if (atributos.count != 0) da_append(&atributos, ';');
                    util_sb_from_cstr(&atributos, row[0]);
                }
                da_append(&atributos, '\0');
            }
            if (!reg_aberto) GuiEnable();

            GuiDrawText("Banco de Dados:", (Rectangle) { 16, 12, 200, 32 }, 0, BLACK);
            if (GuiDropdownBox((Rectangle) { 16, 20 + 16, GetScreenWidth() / 3, 32 }, bancos.items, &bd_indice, db_aberto)) {
                db_aberto = !db_aberto;
                if (!db_aberto) {
                    *query = '\0';
                    banco_selecionado = extract_item(&bancos, bd_indice);
                    sprintf(query, "call listar_tabelas('%.*s')", banco_selecionado.count, banco_selecionado.items);

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

            if (reg_aberto) {
                GuiEnable();
                int offset = 64;

                reg_aberto = !GuiWindowBox((Rectangle) { offset, offset, GetScreenWidth() - 2*offset, GetScreenHeight() - 2*offset}, "Registrar Venda");
                
                char text[5][STR_MAX];
                char label[5][STR_MAX] = {
                    "Filial (Cod.):",
                    "Cliente (Cod.):",
                    "Funcionario (Cod.):",
                    "Forma de Pagamento (Cod.):",
                    "Quantidade (Uni.):"
                };

                for (int i = 0; i < 5; ++i) {
                    GuiLabel((Rectangle) { offset + 32,  offset + (64 - 30) + (i * 64), 256, 30 }, label[i]);
                    GuiEnableTooltip();
                    if (GuiTextBox((Rectangle) { offset + 32, offset + 64 + (i * 64), 256, 30 }, text[i], 24, modo_editar[i])) {
                        modo_editar[i] = !modo_editar[i];
                    }
                }

                // Verifica se existem campos vazios
                for (int i = 0; i < 5; ++i) {
                    if (strcmp(text[i], "") == 0) {
                        GuiDisable();
                        break;
                    }
                }
                if (GuiButton((Rectangle) { offset + 256, offset + 64 + (7 * 64), 128, 30 }, "Registrar")) {
                    *query = '\0';
                    sprintf(query, "call sp_inserir_venda(%s, %s, %s, %s)", text[0], text[1], text[2], text[3]);

                    mysql_next_result(mysql);
                    if (mysql_query(mysql, query)) {
                        fprintf(stderr, "%s\n", mysql_error(mysql));
                        exit(1);
                    }

                    reg_aberto = false;
                }
                
                GuiDisable();
            }
        EndDrawing();
    }

    // Fecha janela
    CloseWindow();

    // Encerra conexao com mysql
    mysql_close(mysql);

    return 0;
}
