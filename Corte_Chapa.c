//Esse mostra o tempo de execucao apos o processo do algoritmo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//Esse e a chapa, e faz com que ela atualize os valores conforme ocorre os cortes
typedef struct{
    int x;        
    int y;       
    int largura;  
    int altura;  
    int tipo;     
}Retangulo;

//Canto da chapa para ser cortada
typedef struct{
    int x;
    int y;
}Canto;

//Define a quantidade de cortes, e o tamanho de cada um deles
typedef struct{
    int largura;
    int altura;
    int max_quantidade; //limite máximo de cortes desse tipo (0 = sem limite)
}TipoCorte;

//Armazena o resultado final de cada execucao
typedef struct{
    int        quantidade_cortes;   
    int        area_utilizada;      //area que foi cortada
    int        area_sobra;          //resto da chapa
    Retangulo *posicoes;            //posicao dos cortes, e o tipo deles
    int       *contagem_por_tipo;   //a quantidade de cortes de cada tipo feitos
}Resultado;

void ler_dimensoes_chapa(int *largura, int *altura);
int ler_quantidade_tipos(void);
void ler_tipos_corte(TipoCorte *tipos, int n_tipos, int chapa_larg, int chapa_alt);
int ler_prioridade(int n_tipos);
int verifica_sobreposicao(Retangulo *cortes, int n_cortes, Retangulo *novo);
int cabe_na_chapa(Retangulo *novo, int chapa_larg, int chapa_alt);
void adicionar_cantos(Canto *cantos, int *n_cantos, int max_cantos, Retangulo *corte);
int canto_ja_existe(Canto *cantos, int n_cantos, int x, int y);
double calcular_grau_ocupacao(Canto c, int larg, int alt,
                                 Retangulo *cortes, int n_cortes,
                                 int chapa_larg, int chapa_alt);
Resultado executar_algoritmo_guloso(int chapa_larg, int chapa_alt,
                                    TipoCorte *tipos, int n_tipos,
                                    int *ordem_prioridade);
void exibir_resultado(Resultado res, int chapa_larg, int chapa_alt,
                            TipoCorte *tipos, int n_tipos, int *ordem_prioridade);
void liberar_resultado(Resultado *res);

int main(void)
{
    int chapa_larg, chapa_alt;
    int n_tipos;
    TipoCorte *tipos;
    int *ordem_prioridade;
    Resultado resultado;

    clock_t inicio, fim;
    double tempo_execucao;
    inicio = clock();

    ler_dimensoes_chapa(&chapa_larg, &chapa_alt);

    n_tipos = ler_quantidade_tipos(); //quantos cortes diferentes o usuario quer

    tipos = (TipoCorte *)malloc(sizeof(TipoCorte) * n_tipos);
    ordem_prioridade = (int *)malloc(sizeof(int) * n_tipos);

    ler_tipos_corte(tipos, n_tipos, chapa_larg, chapa_alt);

    if(n_tipos > 1){
        printf("\nDefina a ordem de prioridade dos cortes.\n");
        printf("O corte prioritario sera tentado primeiro em cada canto;\n");
        printf("se nao couber, tenta o proximo, e assim por diante.\n\n");

        int p;
        for(p = 0; p < n_tipos; p++){
            ordem_prioridade[p] = ler_prioridade(n_tipos);
            printf("Prioridade %d -> Corte %d (%d x %d cm)\n",
                   p + 1,
                   ordem_prioridade[p],
                   tipos[ordem_prioridade[p] - 1].largura,
                   tipos[ordem_prioridade[p] - 1].altura);
        }
    }else{
        ordem_prioridade[0] = 1;
    }

    resultado = executar_algoritmo_guloso(chapa_larg, chapa_alt,
                                          tipos, n_tipos, ordem_prioridade);

    exibir_resultado(resultado, chapa_larg, chapa_alt,
                     tipos, n_tipos, ordem_prioridade);

    fim = clock();
    tempo_execucao = (double)(fim-inicio)/ CLOCKS_PER_SEC;
    printf("Tempo de execucao: %.6f segundos\n", tempo_execucao);

    liberar_resultado(&resultado);
    free(tipos);
    free(ordem_prioridade);

    return 0;
}



void ler_dimensoes_chapa(int *largura, int *altura)
{
    printf("=== SISTEMA DE CORTE DE CHAPA METALICA ===\n\n");
    printf("Informe as dimensoes da chapa de entrada:\n");

    do{
        printf("  Largura da chapa (cm): ");
        scanf("%d", largura);
        if(*largura <= 0)
            printf("  [AVISO] Valor invalido. Digite um numero positivo.\n");
    }while(*largura <= 0);

    do{
        printf("  Altura  da chapa (cm): ");
        scanf("%d", altura);
        if(*altura <= 0)
            printf("  [AVISO] Valor invalido. Digite um numero positivo.\n");
    }while(*altura <= 0);
}

int ler_quantidade_tipos(void)
{
    int n;
    printf("\nInforme a quantidade de tipos de corte diferentes a serem realizados: ");

    do{
        scanf("%d", &n);
        if (n < 1)
            printf("  [AVISO] Informe ao menos 1 tipo de corte: ");
    }while(n < 1);

    return n;
}

void ler_tipos_corte(TipoCorte *tipos, int n_tipos, int chapa_larg, int chapa_alt)
{
    int i;
    for(i = 0; i < n_tipos; i++){
        printf("\nInforme as dimensoes do corte %d:\n", i + 1);

        do{
            do{
                printf("  Largura do corte %d (cm): ", i + 1);
                scanf("%d", &tipos[i].largura);
                if(tipos[i].largura <= 0)
                    printf("  [AVISO] Valor invalido. Digite um numero positivo.\n");
            }while(tipos[i].largura <= 0);

            do{
                printf("  Altura  do corte %d (cm): ", i + 1);
                scanf("%d", &tipos[i].altura);
                if(tipos[i].altura <= 0)
                    printf("  [AVISO] Valor invalido. Digite um numero positivo.\n");
            }while(tipos[i].altura <= 0);

            if(tipos[i].largura > chapa_larg || tipos[i].altura > chapa_alt)
                printf("  [AVISO] O corte (%d x %d) nao cabe na chapa (%d x %d). Tente novamente.\n",
                       tipos[i].largura, tipos[i].altura, chapa_larg, chapa_alt);

        }while(tipos[i].largura > chapa_larg || tipos[i].altura > chapa_alt);

        do{
            printf("  No maximo quantos cortes desse tipo (0 = sem limite): ");
            scanf("%d", &tipos[i].max_quantidade);
            if (tipos[i].max_quantidade < 0)
                printf("  [AVISO] Valor invalido. Digite 0 para sem limite ou um numero positivo.\n");
        }while(tipos[i].max_quantidade < 0);
    }
}

int ler_prioridade(int n_tipos)
{
    int escolha;
    do{
        printf("  Informe o numero do proximo corte prioritario (1 a %d): ", n_tipos);
        scanf("%d", &escolha);
        if (escolha < 1 || escolha > n_tipos)
            printf("  [AVISO] Numero invalido. Escolha entre 1 e %d.\n", n_tipos);
    }while(escolha < 1 || escolha > n_tipos);

    return escolha;
}

/*
 * Verifica se um novo retangulo se sobrepoe a algum corte já realizado
 * Retorna 1 se houver sobreposicao, 0 se estiver livre
 */
int verifica_sobreposicao(Retangulo *cortes, int n_cortes, Retangulo *novo)
{
    int i;
    for(i = 0; i < n_cortes; i++){
        int sep_esq   = (novo->x + novo->largura) <= cortes[i].x;
        int sep_dir   = novo->x >= (cortes[i].x + cortes[i].largura);
        int sep_baixo = (novo->y + novo->altura)  <= cortes[i].y;
        int sep_cima  = novo->y >= (cortes[i].y   + cortes[i].altura);

        if(!(sep_esq || sep_dir || sep_baixo || sep_cima))
            return 1;
    }
    return 0;
}

/*
 * Verifica se um retangulo cabe dentro dos limites da chapa
 * Retorna 1 se couber, 0 se nao
 */
int cabe_na_chapa(Retangulo *novo, int chapa_larg, int chapa_alt)
{
    return (novo->x >= 0) &&
           (novo->y >= 0) &&
           (novo->x + novo->largura <= chapa_larg) &&
           (novo->y + novo->altura  <= chapa_alt);
}

/*
 * Depois de posicionar um corte, dois novos cantos candidatos surgem:
 *   - canto acima: (x, y + altura)
 *   - canto a direita:(x + largura, y)
 */
void adicionar_cantos(Canto *cantos, int *n_cantos, int max_cantos, Retangulo *corte)
{
    int cx1 = corte->x;
    int cy1 = corte->y + corte->altura;
    int cx2 = corte->x + corte->largura;
    int cy2 = corte->y;

    if(!canto_ja_existe(cantos, *n_cantos, cx1, cy1) && *n_cantos < max_cantos){
        cantos[*n_cantos].x = cx1;
        cantos[*n_cantos].y = cy1;
        (*n_cantos)++;
    }

    if(!canto_ja_existe(cantos, *n_cantos, cx2, cy2) && *n_cantos < max_cantos){
        cantos[*n_cantos].x = cx2;
        cantos[*n_cantos].y = cy2;
        (*n_cantos)++;
    }
}

int canto_ja_existe(Canto *cantos, int n_cantos, int x, int y)
{
    int i;
    for(i = 0; i < n_cantos; i++)
        if(cantos[i].x == x && cantos[i].y == y)
            return 1;
    return 0;
}

/*
 * Calcula quantas bordas do corte candidato estão encostadas em algo
 * (parede da chapa ou outro corte ja posicionado).
 * Quanto maior o valor retornado, mais justo é o encaixe.
 */
double calcular_grau_ocupacao(Canto c, int larg, int alt,
                               Retangulo *cortes, int n_cortes,
                               int chapa_larg, int chapa_alt)
{
    int bordas = 0;
    int i;

    if (c.x == 0)
        bordas++;
    if (c.y == 0)                  
        bordas++;
    if (c.x + larg == chapa_larg)  
        bordas++;
    if (c.y + alt  == chapa_alt)   
        bordas++;

    for(i = 0; i < n_cortes; i++){
        if((c.x + larg == cortes[i].x) &&
            !(c.y >= cortes[i].y + cortes[i].altura || c.y + alt <= cortes[i].y))
            bordas++;

        if((c.x == cortes[i].x + cortes[i].largura) &&
            !(c.y >= cortes[i].y + cortes[i].altura || c.y + alt <= cortes[i].y))
            bordas++;

        if((c.y + alt == cortes[i].y) &&
            !(c.x >= cortes[i].x + cortes[i].largura || c.x + larg <= cortes[i].x))
            bordas++;

        if((c.y == cortes[i].y + cortes[i].altura) &&
            !(c.x >= cortes[i].x + cortes[i].largura || c.x + larg <= cortes[i].x))
            bordas++;
    }

    return (double)bordas;
}

Resultado executar_algoritmo_guloso(int chapa_larg, int chapa_alt,
                                    TipoCorte *tipos, int n_tipos,
                                    int *ordem_prioridade)
{
    Resultado res;
    int n_cantos = 0;
    int i, p;
    int pot = 0;
     
    int max_cortes = 0;
    int max_cantos_base = 0;
    for(i = 0; i < n_tipos; i++){
        if(tipos[i].max_quantidade == 0){
            pot = (chapa_larg / tipos[i].largura) * (chapa_alt / tipos[i].altura);
            max_cortes += pot;
        }else
            max_cortes += tipos[i].altura*tipos[i].largura*tipos[i].max_quantidade;
        
        if(pot > max_cantos_base)
            max_cantos_base = pot;
    }
    int max_cantos = 2 * max_cantos_base + 4; 

    Canto *cantos = (Canto *)malloc(sizeof(Canto)* max_cantos);
    res.posicoes = (Retangulo *)malloc(sizeof(Retangulo) * max_cortes);
    res.contagem_por_tipo = (int *)malloc(sizeof(int)* n_tipos);
    res.quantidade_cortes = 0;
    res.area_utilizada = 0;

    if(!cantos || !res.posicoes || !res.contagem_por_tipo){
        printf("[ERRO] Falha ao alocar memoria.\n");
        free(cantos);
        free(res.posicoes);
        free(res.contagem_por_tipo);
        res.quantidade_cortes = 0;
        res.area_utilizada    = 0;
        res.area_sobra        = chapa_larg * chapa_alt;
        return res;
    }

    for(i = 0; i < n_tipos; i++)
        res.contagem_por_tipo[i] = 0;

    cantos[0].x = 0;
    cantos[0].y = 0;
    n_cantos    = 1;

    while(n_cantos > 0){
        int melhor_canto_idx = -1;
        int melhor_tipo_idx  = -1;
        double melhor_grau = -1.0;

        for(i = 0; i < n_cantos; i++){
            for(p = 0; p < n_tipos; p++){
                int t = ordem_prioridade[p] - 1;

                if(tipos[t].max_quantidade > 0 &&
                    res.contagem_por_tipo[t] >= tipos[t].max_quantidade)
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

                double grau = calcular_grau_ocupacao(cantos[i],
                                                      tipos[t].largura,
                                                      tipos[t].altura,
                                                      res.posicoes,
                                                      res.quantidade_cortes,
                                                      chapa_larg, chapa_alt);

                if(grau > melhor_grau){
                    melhor_grau      = grau;
                    melhor_canto_idx = i;
                    melhor_tipo_idx  = p;
                }
            }
        }

        if(melhor_canto_idx == -1)
            break;

        int t_escolhido = ordem_prioridade[melhor_tipo_idx] - 1;

        Retangulo novo_corte;
        novo_corte.x       = cantos[melhor_canto_idx].x;
        novo_corte.y       = cantos[melhor_canto_idx].y;
        novo_corte.largura = tipos[t_escolhido].largura;
        novo_corte.altura  = tipos[t_escolhido].altura;
        novo_corte.tipo    = t_escolhido;

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

void exibir_resultado(Resultado res, int chapa_larg, int chapa_alt,
                      TipoCorte *tipos, int n_tipos, int *ordem_prioridade)
{
    int i, p;
    int area_chapa = chapa_larg * chapa_alt;
    double pct_uso = (res.area_utilizada * 100.0) / area_chapa;
    double pct_sobra = (res.area_sobra * 100.0) / area_chapa;

    printf("\nPosicao de cada corte (canto inferior esquerdo):\n");
    printf("  %-6s %-12s %-10s %-10s\n", "Corte", "Tipo (LxA)", "X (cm)", "Y (cm)");
    printf("  %-6s %-12s %-10s %-10s\n", "-----", "----------", "------", "------");

    for(i = 0; i < res.quantidade_cortes; i++){
        int t = res.posicoes[i].tipo;
        char dim[16];
        sprintf(dim, "%dx%d", tipos[t].largura, tipos[t].altura);
        printf("  %-6d %-12s %-10d %-10d\n",
               i + 1, dim,
               res.posicoes[i].x,
               res.posicoes[i].y);
    }

    printf("\n==========================================\n");
    printf("         RESULTADO DO CORTE\n");
    printf("==========================================\n");
    printf("Chapa de entrada  : %d x %d cm  (area: %d cm2)\n",
           chapa_larg, chapa_alt, area_chapa);

    printf("Tipos de corte    :\n");
    for(i = 0; i < n_tipos; i++){
        int pos_prioridade = -1;
        for(p = 0; p < n_tipos; p++)
            if (ordem_prioridade[p] - 1 == i)
                pos_prioridade = p + 1;

        char limite_str[20];
        if(tipos[i].max_quantidade > 0)
            sprintf(limite_str, "%d", tipos[i].max_quantidade);
        else
            sprintf(limite_str, "sem limite");
        printf("  Corte %d: %d x %d cm  (prioridade %d, limite: %s)\n",
               i + 1, tipos[i].largura, tipos[i].altura, pos_prioridade, limite_str);
    }

    printf("------------------------------------------\n");
    printf("Cortes realizados : %d peca(s) no total\n", res.quantidade_cortes);

    for(i = 0; i < n_tipos; i++)
        printf("  Corte %d (%dx%d cm): %d peca(s)%s\n",
               i + 1, tipos[i].largura, tipos[i].altura,
               res.contagem_por_tipo[i],
               tipos[i].max_quantidade > 0 ? "" : "  (sem limite)");

    printf("Area utilizada    : %d cm2  (%.2f%%)\n", res.area_utilizada, pct_uso);
    printf("------------------------------------------\n");

    if(res.area_sobra > 0)
        printf("Sobra de material : %d cm2  (%.2f%%)\n", res.area_sobra, pct_sobra);
    else
        printf("Sobra de material : Nenhuma (aproveitamento total!)\n");

    printf("==========================================\n\n");
}

void liberar_resultado(Resultado *res)
{
    if (res->posicoes != NULL) {
        free(res->posicoes);
        res->posicoes = NULL;
    }
    if (res->contagem_por_tipo != NULL) {
        free(res->contagem_por_tipo);
        res->contagem_por_tipo = NULL;
    }
}
