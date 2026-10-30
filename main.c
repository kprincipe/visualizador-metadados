#include <stdio.h>
#include <stdlib.h>

#include <mysql/mysql.h>

#include "raylib.h"

// TODO: Achar uma maneira de nao precisar alocar esse espaco estaticamente
#define RAYGUI_TEXTSPLIT_MAX_ITEMS          128 * 4
#define RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE     1024 * 4

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define SV_IMPLEMENTATION
#include <sv.h>

#define STR_MAX 1024 * 8

typedef struct nodo_t {
    char nome[STR_MAX];
    struct nodo_t *proximo;
} nodo_t;

typedef struct {
    Rectangle posicao;
} combobox_estado;

void anexar_nodo(nodo_t *inicio, char *s) {
    nodo_t *temp = inicio;
    
    nodo_t *novo = malloc(sizeof(nodo_t));
    sprintf(novo->nome, "%s", s);
    novo->proximo = NULL;

    while (temp->proximo != NULL) {
        temp = temp->proximo;
    }

    temp->proximo = novo;
}

void liberar_nodos(nodo_t *inicio) {
    nodo_t *temp = inicio->proximo;
    inicio->proximo = NULL;

    while (inicio->proximo != NULL) {
        temp = inicio->proximo;
        inicio = inicio->proximo;
        free(temp);
    }
}

nodo_t *buscar_nodo_indice(nodo_t *inicio, size_t indice) {
    nodo_t *temp = inicio;
    
    int c = 0;
    while (temp->proximo != NULL) {
        temp = temp->proximo;
        if (c == indice) break;
        c++;
    }
    return temp;
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
    
    int qtde_bancos = 0;
    int qtde_tabelas = 0;

    char bancos_combobox[STR_MAX] = " ";
    char tabelas_combobox[STR_MAX] = " ";

    nodo_t bancos = { .proximo = NULL };
    nodo_t tabelas = { .proximo = NULL };

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
        anexar_nodo(&bancos, row[0]);
        qtde_bancos += 1;
    }

    mysql_free_result(res);

    nodo_t *tmp = &bancos;
    while (tmp->proximo != NULL) {
        tmp = tmp->proximo;

        strcat(bancos_combobox, tmp->nome);
        if (tmp->proximo != NULL)
            strcat(bancos_combobox, ";");
    }

    // Inicializa janela
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Information Schema Mysql");

    int bd_selecionado = 0;
    int tb_ativo = 0;

    int listViewScrollIndex = 0;
    int listViewActive = -1;

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

            //DrawText("Tabela:", 16, 16 + 64, GuiGetFont().baseSize, BLACK);
            GuiDrawText("Tabela:", (Rectangle) { 16, 12 + 64, 200, 32 }, 0, BLACK);
        
            tabelas_estado.posicao.height = GetScreenHeight() - 128;
            
            if (db_aberto) GuiDisable();
            
            if (GuiListView(tabelas_estado.posicao, tabelas_combobox, &listViewScrollIndex, &listViewActive)) {
                nodo_t *tmp = buscar_nodo_indice(&tabelas, listViewActive);
                printf("DEBUG: tabela %s\n", tmp->nome);
            }

            GuiDrawText("Banco de Dados:", (Rectangle) { 16, 12, 200, 32 }, 0, BLACK);
            GuiEnable();

            // TODO: Lidar com sv_to_cstr, aqui ocorre um memory leak
            if (GuiDropdownBox(bancos_estado.posicao, bancos_combobox, &bd_selecionado, db_aberto)) {
                db_aberto = !db_aberto;
                if (!db_aberto) {
                    nodo_t *bd_ativo = buscar_nodo_indice(&bancos, bd_selecionado);

                    *query = '\0';
                    sprintf(query, "call listar_tabelas('%s')", bd_ativo->nome);

                    mysql_next_result(mysql);
                    if (mysql_query(mysql, query)) {
                        fprintf(stderr, "%s\n", mysql_error(mysql));
                        exit(1);
                    }
                        
                    res = mysql_use_result(mysql);

                    liberar_nodos(&tabelas);
                    tabelas.proximo = NULL;
                    while ((row = mysql_fetch_row(res)) != NULL) {
                        anexar_nodo(&tabelas, row[0]);
                        printf("%s: %s\n", bd_ativo->nome, row[0]);
                    }

                    nodo_t *tmp = &tabelas;
                    *tabelas_combobox = '\0';
                    while (tmp->proximo != NULL) {
                        tmp = tmp->proximo;

                        if ((strlen(tabelas_combobox) + strlen(tmp->nome) + 1) <= STR_MAX) {
                            
                            strcat(tabelas_combobox, tmp->nome);
                            if (tmp->proximo != NULL)
                                strcat(tabelas_combobox, ";");
                        } else {
                            fprintf(stderr, "ERRO: tabelas_combobox atingiu seu limite\n");
                        }
                    }
                    printf("DEBUG: %s\n", tabelas_combobox);
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
