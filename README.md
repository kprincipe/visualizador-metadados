# Visualizador de Metadados
> [!WARNING]
> Em estágio preliminar de desenvolvimento!

Este visualizador de metadados tem como objetivo ser uma interface para visualização de tabelas e relacionamentos de banco de dados MySQL. O projeto foi desenvolvido para a obtenção de nota na disciplina de Administração e Programação de Banco de Dados.

## Compilação e Execução
O projeto depende apenas de um biblioteca gráfica chamada _raylib_ disponível em repositórios da maioria das distribuições linux.

**Fedora**:
```bash
dnf install raylib
dnf install raylib-devel
```

**Arch Linux**:
```bash
pacman -S raylib
```

**Debian**:
```Debian
sudo apt install libraylib-dev
sudo apt install libraylib550
```

**Outras**:<br>
[Consultar Documentação Oficial](https://github.com/raysan5/raylib#build-and-installation)

Após a instalação da biblioteca _raylib_, basta compilar utilizando o script de compilação _Makefile_ e executar o binário gerado:

```bash
make
./visualizador
```

## Objetivos
- [x] Listar todos os bancos de dados;
- [ ] Exibir tabelas de um banco de dados específico;
- [ ] Listar colunas de uma tabela específica.
