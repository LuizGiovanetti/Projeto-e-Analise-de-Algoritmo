/*
   Algoritmo Guloso para Corte de Chapa Metalica

   Baseado no artigo:
   "Greedy Algorithm for Rectangle-packing Problem"
   Chen Duanbing, Huang Wenqi (2007)

   Paradigma: Algoritmo Guloso (Greedy)
   Estrategia: Acao de Ocupar Cantos (Corner-Occupying Action)

   Formato do arquivo de instancias:
     @INSTANCIA  <nome> <chapa_larg> <chapa_alt>
     @ARTIGO     <n_pecas_ref> <sobra%_ref> <tempo_s_ref>
     @TIPOS      <n_tipos>
     <largura> <altura> <max_qtd>    <- uma linha por tipo; max_qtd=0 sem limite
     ...
     @PRIORIDADE <p1> <p2> ... <pN>  <- indices 1..N na ordem de prioridade
     ---                             <- separador obrigatorio entre instancias
     # comentario                    <- linhas com # sao ignoradas
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct{
    int x, y;
    int largura, altura;
    int tipo;
}Retangulo;

typedef struct{
    int x, y;
}Canto;

typedef struct{
    int largura, altura;
    int max_quantidade; // 0 = sem limite
}TipoCorte;

typedef struct{
    int quantidade_cortes;
    int area_utilizada;
    int area_sobra;
    Retangulo *posicoes;
    int *contagem_por_tipo;
}Resultado;

typedef struct{
    char nome[32];
    int  chapa_larg, chapa_alt;
    int  n_tipos;
    TipoCorte *tipos;
    int *prioridade; //vetor com a ordem de prioridade lida do arquivo

    //Resultados do artigo - 0 quando nao ha referencia
    int    n_pecas_artigo;
    double sobra_artigo;
    double tempo_artigo;

    //Resultado apos a execucao
    int    cortes_nosso;
    int    area_chapa;
    int    area_utilizada;
    int    area_sobra;
    double sobra_nossa;
    double tempo_nosso;
}Instancia;

int verifica_sobreposicao(Retangulo *cortes, int n, Retangulo *novo);
int cabe_na_chapa(Retangulo *novo, int larg, int alt);
void adicionar_cantos(Canto *cantos, int *n_cantos, int max_cantos, Retangulo *corte);
int canto_ja_existe(Canto *cantos, int n_cantos, int x, int y);
double calcular_grau_ocupacao(Canto c, int larg, int alt, Retangulo *cortes, int n_cortes, int chapa_larg, int chapa_alt);
Resultado executar_algoritmo_guloso(int chapa_larg, int chapa_alt, TipoCorte *tipos, int n_tipos, int *ordem_prioridade);
void exibir_resultado_instancia(Instancia *d, Resultado res);
void exibir_tabela_comparativa(Instancia *inst, int n);
void liberar_resultado(Resultado *res);
Instancia *ler_instancias(const char *caminho, int *n_instancias);
void liberar_instancias(Instancia *inst, int n);
static char *ler_linha_dinamica(FILE *f);
void salvar_cortes_txt(Instancia *d, Resultado res, FILE *f_saida);

int main(int argc, char *argv[]){
    const char *arquivo = (argc >= 2) ? argv[1] : "instancias.txt";

    int n_instancias = 0;
    Instancia *instancias = ler_instancias(arquivo, &n_instancias);

    if((!instancias) || (n_instancias == 0)){
        fprintf(stderr, "[ERRO] Nenhuma instancia carregada de \"%s\".\n", arquivo);
        return 1;
    }

    printf("=== SISTEMA DE CORTE DE CHAPA METALICA ===\n");
    printf("Referencia: Chen & Huang (2007) - Tabela 1\n");
    printf("Maquina do artigo: Pentium 4 IBM R40 (2.0GHz, 256MB RAM)\n");
    printf("Instancias lidas de: %s  (%d instancia(s))\n\n", arquivo, n_instancias);

    FILE *f_cortes = fopen("cortes_saida.txt", "w");
    if(!f_cortes)
        fprintf(stderr, "[AVISO] Nao foi possivel criar cortes_saida.txt. Continuando sem salvar.\n");

    int inst;
    for(inst = 0; inst < n_instancias; inst++){
        Instancia *d = &instancias[inst];

        printf("==========================================\n");
        printf(" Instancia: %s | Chapa: %dx%d cm | %d tipos\n",
               d->nome, d->chapa_larg, d->chapa_alt, d->n_tipos);
        printf("==========================================\n");

        d->area_chapa = d->chapa_larg * d->chapa_alt;

        clock_t inicio = clock();
        Resultado res = executar_algoritmo_guloso(d->chapa_larg, d->chapa_alt, d->tipos, d->n_tipos, d->prioridade);
        clock_t fim = clock();
        d->tempo_nosso = (double)(fim - inicio) / CLOCKS_PER_SEC;

        d->cortes_nosso = res.quantidade_cortes;
        d->area_utilizada = res.area_utilizada;
        d->area_sobra = res.area_sobra;
        d->sobra_nossa = (res.area_sobra * 100.0) / d->area_chapa;

        exibir_resultado_instancia(d, res);

        if(f_cortes)
            salvar_cortes_txt(d, res, f_cortes);

        liberar_resultado(&res);
    }

    if(f_cortes)
        fclose(f_cortes);

    exibir_tabela_comparativa(instancias, n_instancias);

    liberar_instancias(instancias, n_instancias);
    return 0;
}

void salvar_cortes_txt(Instancia *d, Resultado res, FILE *f_saida){
    int i;
 
    //cabecalho da instancia: nome e dimensoes da chapa 
    fprintf(f_saida, "INSTANCIA %s %d %d\n", d->nome, d->chapa_larg, d->chapa_alt);
 
    //uma linha por corte realizado: canto inf-esq e dimensoes
    for(i = 0; i < res.quantidade_cortes; i++){
        Retangulo *r = &res.posicoes[i];
        fprintf(f_saida, "CORTE %d %d %d %d\n",
                r->x, r->y, r->largura, r->altura);
    }

    fprintf(f_saida, "FIM\n");
}

static char *ler_linha_dinamica(FILE *f){
    size_t cap = 128;
    size_t len = 0;
    char *buf = (char*)malloc(cap);

    if(!buf) 
        return NULL;
 
    int c;
    while((c = fgetc(f)) != EOF){
        if((len + 1) >= cap){           //sem espaco: dobra o buffer
            cap *= 2;
            char *tmp = (char*)realloc(buf, cap);
            if(!tmp){
                free(buf); 
                return NULL; 
            }
            buf = tmp;
        }
        if(c == '\n') 
            break;           //fim da linha Unix/Windows 
        if(c == '\r') 
            continue;        //*descarta CR do Windows  
        buf[len++] = (char)c;
    }
 
    if((len == 0) && (c == EOF)){ 
        free(buf); 
        return NULL; 
    }
 
    buf[len] = '\0';
    return buf;
}
 
Instancia *ler_instancias(const char *caminho, int *n_instancias){
    FILE *f = fopen(caminho, "r");
    if(!f){
        fprintf(stderr, "[ERRO] Nao foi possivel abrir \"%s\".\n", caminho);
        *n_instancias = 0;
        return NULL;
    }
 
    //conta instancias pelo numero de ---
    int cap = 0;
    char *linha;
    while((linha = ler_linha_dinamica(f)) != NULL){
        if((linha[0] != '#') && (linha[0] != '\0') && (strncmp(linha, "---", 3) == 0))
            cap++;
        free(linha);
    }
 
    if(cap == 0){
        fclose(f);
        *n_instancias = 0;
        return NULL;
    }
 
    Instancia *vetor = (Instancia*)calloc(cap, sizeof(Instancia));
    if(!vetor){
        fclose(f);
        *n_instancias = 0;
        return NULL;
    }
 
    //preenche
    rewind(f);
    int idx = -1;        //indice da instancia atual, incrementado em cada ---
    int tipos_lidos = 0;
 
    while((linha = ler_linha_dinamica(f)) != NULL){
 
        if((linha[0] == '#') || (linha[0] == '\0')){
            free(linha); 
            continue;
        }
 
        // ---  encerra a instancia em construcao
        if(strncmp(linha, "---", 3) == 0)
        {
            idx++;
            tipos_lidos = 0;
            free(linha); continue;
        }
 
        int prox = idx + 1; //monta a instancia
        if(prox >= cap){
            free(linha); 
            continue;
        }
 
        // @INSTANCIA <nome> <larg> <alt>
        if(strncmp(linha, "@INSTANCIA", 10) == 0){
            sscanf(linha + 10, " %31s %d %d", vetor[prox].nome, &vetor[prox].chapa_larg, &vetor[prox].chapa_alt);
            free(linha); 
            continue;
        }
 
        // @ARTIGO <n_pecas> <sobra%> <tempo_s>
        if(strncmp(linha, "@ARTIGO", 7) == 0){
            sscanf(linha + 7, " %d %lf %lf", &vetor[prox].n_pecas_artigo, &vetor[prox].sobra_artigo, &vetor[prox].tempo_artigo);
            free(linha); continue;
        }
 
        // @TIPOS <n> : aloca tipos e prioridade 
        if(strncmp(linha, "@TIPOS", 6) == 0){
            sscanf(linha + 6, " %d", &vetor[prox].n_tipos);
 
            vetor[prox].tipos = (TipoCorte*)malloc(sizeof(TipoCorte) * vetor[prox].n_tipos);
            vetor[prox].prioridade = (int*)malloc(sizeof(int) * vetor[prox].n_tipos);
 
            if(!vetor[prox].tipos || !vetor[prox].prioridade){
                fprintf(stderr, "[ERRO] Falha ao alocar instancia %d.\n", prox);
                free(linha);
                fclose(f);
                liberar_instancias(vetor, prox);
                *n_instancias = 0;
                return NULL;
            }
 
            //Prioridade padrao sequencial, substitui se @PRIORIDADE aparecer
            int k;
            for(k = 0; k < vetor[prox].n_tipos; k++)
                vetor[prox].prioridade[k] = k + 1;
 
            tipos_lidos = 0;
            free(linha); continue;
        }
 
        //@PRIORIDADE <p1> <p2> ... <pN>
        if(strncmp(linha, "@PRIORIDADE", 11) == 0){
            char *ptr = linha + 11;
            int k;
            for(k = 0; k < vetor[prox].n_tipos; k++){
                while((*ptr == ' ') || (*ptr == '\t')) 
                    ptr++;

                if(*ptr == '\0') 
                    break;

                vetor[prox].prioridade[k] = atoi(ptr);

                while((*ptr != ' ') && (*ptr != '\t') && (*ptr != '\0'))
                    ptr++;
            }
            free(linha); 
            continue;
        }
 
        //Linha de tipo: <largura> <altura> <max_qtd> 
        if(vetor[prox].tipos && (tipos_lidos < vetor[prox].n_tipos)){
            TipoCorte *t = &vetor[prox].tipos[tipos_lidos];

            if(sscanf(linha, " %d %d %d", &t->largura, &t->altura, &t->max_quantidade) == 3)
                tipos_lidos++;
        }
        free(linha);
    }
 
    fclose(f);
    *n_instancias = cap;
    return vetor;
}
void liberar_instancias(Instancia *inst, int n){
    int i;
    for(i = 0; i < n; i++){
        free(inst[i].tipos);
        free(inst[i].prioridade);
    }
    free(inst);
}

int verifica_sobreposicao(Retangulo *cortes, int n_cortes, Retangulo *novo){
    int i;
    for(i = 0; i < n_cortes; i++){
        int sep_esq = (novo->x + novo->largura) <= cortes[i].x;
        int sep_dir =  novo->x >= (cortes[i].x + cortes[i].largura);
        int sep_baixo = (novo->y + novo->altura)  <= cortes[i].y;
        int sep_cima =  novo->y >= (cortes[i].y  + cortes[i].altura);
        if(!(sep_esq || sep_dir || sep_baixo || sep_cima))
            return 1;
    }
    return 0;
}

int cabe_na_chapa(Retangulo *novo, int chapa_larg, int chapa_alt){
    return(novo->x >= 0) && (novo->y >= 0) && (novo->x + novo->largura <= chapa_larg) && (novo->y + novo->altura  <= chapa_alt);
}

//depois de posicionar um corte, surgem dois novos cantos: (x, y + altura)-> canto acima E (x + largura, y)-> canto a direita

void adicionar_cantos(Canto *cantos, int *n_cantos, int max_cantos, Retangulo *corte){
    int cx1 = corte->x, cy1 = corte->y + corte->altura;
    int cx2 = corte->x + corte->largura, cy2 = corte->y;

    if(!canto_ja_existe(cantos, *n_cantos, cx1, cy1) && (*n_cantos < max_cantos)){
        cantos[*n_cantos].x = cx1;
        cantos[*n_cantos].y = cy1;
        (*n_cantos)++;
    }
    if(!canto_ja_existe(cantos, *n_cantos, cx2, cy2) && (*n_cantos < max_cantos)){
        cantos[*n_cantos].x = cx2;
        cantos[*n_cantos].y = cy2;
        (*n_cantos)++;
    }
}

int canto_ja_existe(Canto *cantos, int n_cantos, int x, int y){
    int i;
    for(i = 0; i < n_cantos; i++)
        if((cantos[i].x == x) && (cantos[i].y == y))
            return 1;
    return 0;
}

//verifica qual o tipo de corte que encosta em mais bordas. Qual tiver maior valor quer dizer que e a melhor escolha gulosa, pois e mais justo
double calcular_grau_ocupacao(Canto c, int larg, int alt, Retangulo *cortes, int n_cortes, int chapa_larg, int chapa_alt){
    int bordas = 0, i;

    if(c.x == 0)
        bordas++;

    if(c.y == 0)
        bordas++;

    if(c.x + larg == chapa_larg)  
        bordas++;

    if(c.y + alt  == chapa_alt)   
        bordas++;

    for(i = 0; i < n_cortes; i++){
        int sy = !((c.y >= cortes[i].y + cortes[i].altura) || (c.y + alt <= cortes[i].y));
        int sx = !((c.x >= cortes[i].x + cortes[i].largura) || (c.x + larg <= cortes[i].x));

        if((c.x + larg == cortes[i].x) && sy) 
            bordas++;

        if((c.x == cortes[i].x + cortes[i].largura) && sy) 
            bordas++;

        if((c.y + alt  == cortes[i].y) && sx) 
            bordas++;

        if((c.y == cortes[i].y + cortes[i].altura) && sx) 
            bordas++;
    }

    return (double)bordas;
}

Resultado executar_algoritmo_guloso(int chapa_larg, int chapa_alt, TipoCorte *tipos, int n_tipos, int *ordem_prioridade){
    Resultado res;
    int n_cantos = 0, i, p;

    int max_cortes = 0, max_cantos_base = 0;
    for(i = 0; i < n_tipos; i++){
        int pot = (chapa_larg / tipos[i].largura) * (chapa_alt / tipos[i].altura);
        max_cortes += (tipos[i].max_quantidade > 0) ? tipos[i].max_quantidade : pot;

        if(pot > max_cantos_base) 
            max_cantos_base = pot;
    }
    int max_cantos = 2 * max_cantos_base + 4;

    Canto *cantos = (Canto*)malloc(sizeof(Canto) * max_cantos);
    res.posicoes = (Retangulo*)malloc(sizeof(Retangulo) * (max_cortes + 1));
    res.contagem_por_tipo = (int*)malloc(sizeof(int) * n_tipos);
    res.quantidade_cortes = 0;
    res.area_utilizada = 0;

    if(!cantos || !res.posicoes || !res.contagem_por_tipo){
        printf("[ERRO] Falha ao alocar memoria.\n");
        free(cantos); 
        free(res.posicoes); 
        free(res.contagem_por_tipo);
        res.quantidade_cortes = 0;
        res.area_utilizada = 0;
        res.area_sobra = chapa_larg * chapa_alt;
        return res;
    }

    for(i = 0; i < n_tipos; i++)
        res.contagem_por_tipo[i] = 0;

    cantos[0].x = 0; 
    cantos[0].y = 0; 
    n_cantos = 1;

    while(n_cantos > 0){
        int melhor_canto_idx = -1;
        int melhor_tipo_idx  = -1;
        double melhor_grau = -1.0;

        for(i = 0; i < n_cantos; i++){
            for(p = 0; p < n_tipos; p++){
                int t = ordem_prioridade[p] - 1;

                if((tipos[t].max_quantidade > 0) && (res.contagem_por_tipo[t] >= tipos[t].max_quantidade))
                    continue;

                Retangulo candidato;
                candidato.x = cantos[i].x;
                candidato.y = cantos[i].y;
                candidato.largura = tipos[t].largura;
                candidato.altura = tipos[t].altura;
                candidato.tipo = t;

                if(!cabe_na_chapa(&candidato, chapa_larg, chapa_alt))              
                    continue;

                if(verifica_sobreposicao(res.posicoes, res.quantidade_cortes, &candidato))
                    continue;

                double grau = calcular_grau_ocupacao(cantos[i], tipos[t].largura, tipos[t].altura, res.posicoes, res.quantidade_cortes, chapa_larg, chapa_alt);

                if(grau > melhor_grau){
                    melhor_grau = grau;
                    melhor_canto_idx = i;
                    melhor_tipo_idx  = p;
                }
            }
        }

        if(melhor_canto_idx == -1) 
            break;

        int t_escolhido = ordem_prioridade[melhor_tipo_idx] - 1;

        Retangulo novo_corte;
        novo_corte.x = cantos[melhor_canto_idx].x;
        novo_corte.y = cantos[melhor_canto_idx].y;
        novo_corte.largura = tipos[t_escolhido].largura;
        novo_corte.altura = tipos[t_escolhido].altura;
        novo_corte.tipo = t_escolhido;

        res.posicoes[res.quantidade_cortes] = novo_corte;
        res.quantidade_cortes++;
        res.area_utilizada += novo_corte.largura * novo_corte.altura;
        res.contagem_por_tipo[t_escolhido]++;

        cantos[melhor_canto_idx] = cantos[n_cantos - 1];
        n_cantos--;
        adicionar_cantos(cantos, &n_cantos, max_cantos, &novo_corte);
    }

    res.area_sobra = (chapa_larg * chapa_alt) - res.area_utilizada;
    free(cantos);
    return res;
}

void exibir_resultado_instancia(Instancia *d, Resultado res){
    int i, p;
    double pct_uso = (res.area_utilizada * 100.0) / d->area_chapa;
    double pct_sobra = (res.area_sobra * 100.0) / d->area_chapa;

    printf("\nPosicao de cada corte (canto inferior esquerdo):\n");
    printf("  %-6s %-12s %-10s %-10s\n", "Corte", "Tipo(LxA)", "X(cm)", "Y(cm)");
    printf("  %-6s %-12s %-10s %-10s\n", "-----", "---------", "-----", "-----");

    for(i = 0; i < res.quantidade_cortes; i++){
        int t = res.posicoes[i].tipo;
        char dim[16];
        sprintf(dim, "%dx%d", d->tipos[t].largura, d->tipos[t].altura);
        printf("  %-6d %-12s %-10d %-10d\n", i + 1, dim, res.posicoes[i].x, res.posicoes[i].y);
    }

    printf("\n------------------------------------------\n");
    printf("  Chapa          : %d x %d cm  (area: %d cm2)\n", d->chapa_larg, d->chapa_alt, d->area_chapa);
    printf("  Tipos definidos: %d\n", d->n_tipos);
    printf("  Cortes feitos  : %d peca(s)\n\n", res.quantidade_cortes);

    for(i = 0; i < d->n_tipos; i++){
        int pos_pri = -1;

        for(p = 0; p < d->n_tipos; p++){
            if(d->prioridade[p] - 1 == i){ 
                pos_pri = p + 1; 
                break; 
            }
        }

        char limite_str[20];
        if(d->tipos[i].max_quantidade > 0)
            sprintf(limite_str, "max %d", d->tipos[i].max_quantidade);
        else
            sprintf(limite_str, "sem limite");

        printf("  Tipo %d (%dx%d cm, prior.%d, %s): %d peca(s)\n", i + 1, d->tipos[i].largura, d->tipos[i].altura, pos_pri, limite_str, res.contagem_por_tipo[i]);
    }

    printf(" Area utilizada : %d cm2  (%.2f%%)\n", res.area_utilizada, pct_uso);
    if(res.area_sobra > 0)
        printf(" Sobra          : %d cm2  (%.2f%%)\n", res.area_sobra, pct_sobra);
    else
        printf("  Sobra          : Nenhuma (aproveitamento total!)\n");

    printf("  Tempo algoritmo: %.6f segundos\n", d->tempo_nosso);
    printf("------------------------------------------\n\n");
}

void exibir_tabela_comparativa(Instancia *inst, int n){
    int i;

    printf("\n");
    printf("+-------+----------+--------+--------+-----------+-----------+----------+----------+--------------+--------------+\n");
    printf("| %-5s | %-8s | %-6s | %-6s | %-9s | %-9s | %-8s | %-8s | %-12s | %-12s |\n", "Inst.", "Chapa", "Ref.", "Nosso", "Area cm2", "Usada cm2", "Sobra%Ref", "Sobra%Nos", "Tempo Ref(s)", "Tempo Nos(s)");
    printf("+-------+----------+--------+--------+-----------+-----------+----------+----------+--------------+--------------+\n");

    for(i = 0; i < n; i++){
        Instancia *d = &inst[i];
        char chapa_str[16];
        sprintf(chapa_str, "%dx%d", d->chapa_larg, d->chapa_alt);
        printf("| %-5s | %-8s | %-6d | %-6d | %-9d | %-9d | %-8.2f | %-8.2f | %-12.2f | %-12.6f |\n", d->nome, chapa_str, d->n_pecas_artigo, d->cortes_nosso, d->area_chapa, d->area_utilizada, d->sobra_artigo, d->sobra_nossa, d->tempo_artigo, d->tempo_nosso);
    }

    printf("+-------+----------+--------+--------+-----------+-----------+----------+----------+--------------+--------------+\n");

    //instancias com referencia do artigo 
    printf("\nAvaliacao (instancias com referencia do artigo):\n");
    int tem = 0;
    for(i = 0; i < n; i++){
        Instancia *d = &inst[i];

        if(d->n_pecas_artigo == 0) 
            continue;

        tem = 1;
        printf("  %s -> Sobra: %s (%.2f%% nosso vs %.2f%% artigo) | Tempo: %s (%.6fs nosso vs %.2fs artigo)\n", d->nome, d->sobra_nossa <= d->sobra_artigo ? "IGUAL/MELHOR" : "PIOR", d->sobra_nossa, d->sobra_artigo, d->tempo_nosso <= d->tempo_artigo ? "MAIS RAPIDO" : "MAIS LENTO", d->tempo_nosso, d->tempo_artigo);
    }

    if(!tem)
        printf("  Nenhuma instancia com referencia de artigo encontrada.\n");

    printf("\n");
}

void liberar_resultado(Resultado *res){
    if (res->posicoes){ 
        free(res->posicoes);
        res->posicoes = NULL; 
    }
    
    if(res->contagem_por_tipo){ 
        free(res->contagem_por_tipo); 
        res->contagem_por_tipo = NULL; 
    }
}