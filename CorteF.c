/*
   Algoritmo Guloso para Corte de Chapa Metalica
  
   Baseado no artigo:
   "Greedy Algorithm for Rectangle-packing Problem"
   Chen Duanbing, Huang Wenqi (2007)
 
   Paradigma: Algoritmo Guloso (Greedy)
   Estrategia: Acao de Ocupar Cantos (Corner-Occupying Action)
 */
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

//Resultado final do algoritmo
typedef struct{
    int quantidade_cortes;
    int area_utilizada;
    int area_sobra;
    Retangulo *posicoes;
    int *contagem_por_tipo;
}Resultado;

/*
   Guarda os dados de uma instancia (chapa + tipos de corte)
   e os resultados do artigo para comparacao ao final.
 */
typedef struct{
    const char *nome;   // identificador da instancia        
    int chapa_larg;     // largura da chapa                  
    int chapa_alt;      // altura da chapa                   
    int n_tipos;        // numero de tipos de corte          
    TipoCorte *tipos;   // vetor de tipos de corte           

    //Resultados do artigo (Tabela 1 - Chen & Huang, 2007)        
    int n_pecas_artigo;      //n_tipos da instancia no artigo    
    double sobra_artigo;     // % area nao utilizada no artigo    
    double tempo_artigo;     // tempo CPU (s) relatado no artigo  

    // Resultados do nosso algoritmo (preenchidos apos execucao)     
    int cortes_nosso;           // cortes efetivamente realizados    
    int area_chapa;             // area total da chapa               
    int area_utilizada;         // area coberta pelos cortes         
    int area_sobra;             // area restante                     
    double sobra_nossa;         // % de area nao utilizada           
    double tempo_nosso;         // tempo CPU (s) so do algoritmo     
}Instancia;

// C11: chapa 20x20, 16 pecas, area total = 400 cm2 
static TipoCorte tipos_C11[] = {
    { 2, 5,1},{ 13, 5,1},{ 4,11,1},{ 11,11,1},
    { 2, 1,1},{ 2, 3,1},{ 6, 4,1},{ 7, 4,1},
    { 2, 2,1},{ 3, 2,1},{ 5, 3,1},{ 5, 4,1},
    { 3, 4,1},{ 2, 4,1},{ 5, 5,1},{ 5, 2,1}
};

// C21: chapa 40x15, 25 pecas, area total = 600 cm2
static TipoCorte tipos_C21[] = {
    { 4, 3,1},{ 1, 1,1},{ 1, 2,1},{ 4, 3,1},{ 2, 2,1},
    { 2, 1,1},{ 7, 7,1},{ 4, 6,1},{ 4, 1,1},{ 4, 5,1},
    { 1, 5,1},{ 6, 5,1},{13,15,1},{10,11,1},{10, 4,1},
    { 2,12,1},{ 2, 1,1},{ 2, 2,1},{ 4, 6,1},{ 1, 1,1},
    { 3, 1,1},{ 4, 1,1},{ 4, 5,1},{ 3, 2,1},{ 1, 2,1}
};

// C41: chapa 60x60, 49 pecas, area total = 3600 cm2
static TipoCorte tipos_C41[] = {
    { 5, 4,1},{ 2, 4,1},{ 2, 4,1},{ 9,10,1},{ 4, 2,1},
    { 5, 2,1},{ 9, 3,1},{ 9, 4,1},{ 9,10,1},{ 9,11,1},
    { 9, 8,1},{ 9, 4,1},{ 3, 3,1},{ 1, 1,1},{ 2, 1,1},
    { 1, 2,1},{ 1, 1,1},{ 1, 1,1},{ 1, 2,1},{ 1, 1,1},
    { 1, 1,1},{ 1, 4,1},{ 3, 3,1},{ 3, 1,1},{10,13,1},
    { 4, 3,1},{ 6, 3,1},{ 6, 6,1},{ 6, 8,1},{ 6, 2,1},
    { 6, 6,1},{ 6, 8,1},{ 6, 2,1},{ 8,16,1},{19,16,1},
    { 2,16,1},{21,37,1},{19,30,1},{19, 7,1},{15, 7,1},
    {19, 7,1},{ 6, 7,1},{11, 8,1},{11,10,1},{11, 3,1},
    {11,10,1},{11, 5,1},{ 8, 8,1},{ 3, 8,1}
};

int verifica_sobreposicao(Retangulo *cortes, int n_cortes, Retangulo *novo);
int cabe_na_chapa(Retangulo *novo, int chapa_larg, int chapa_alt);
//int cmp_area_desc(const void *a, const void *b);
//void ordenar_por_area_desc(TipoCorte *tipos, int n_tipos);
void adicionar_cantos(Canto *cantos, int *n_cantos, int max_cantos, Retangulo *corte);
int canto_ja_existe(Canto *cantos, int n_cantos, int x, int y);
double calcular_grau_ocupacao(Canto c, int larg, int alt, Retangulo *cortes, int n_cortes, int chapa_larg, int chapa_alt);
Resultado executar_algoritmo_guloso(int chapa_larg, int chapa_alt, TipoCorte *tipos, int n_tipos, int *ordem_prioridade);
void exibir_resultado_instancia(Instancia *d, Resultado res, int *ordem_prioridade);
void exibir_tabela_comparativa(Instancia *inst, int n);
void liberar_resultado(Resultado *res);

int main(void)
{
    Instancia instancias[] = {
        //nome  larg  alt  n   tipos   npecas sobra% tempo(s) 
        {"C11",  20,  20,  16, tipos_C11, 16, 0.00, 0.04, 0, 0, 0, 0, 0.0, 0.0},
        {"C21",  40,  15,  25, tipos_C21, 25, 0.00, 0.51, 0, 0, 0, 0, 0.0, 0.0},
        {"C41",  60,  60,  49, tipos_C41, 49, 0.33, 3.57, 0, 0, 0, 0, 0.0, 0.0}
    };
    const int n_instancias = 3;

    printf("=== SISTEMA DE CORTE DE CHAPA METALICA ===\n");
    printf("Referencia: Chen & Huang (2007) - Tabela 1\n");
    printf("Maquina do artigo: Pentium 4 IBM R40 (2.0GHz, 256MB RAM)\n\n");

    //Processa cada instancia 
    int inst;
    for(inst = 0; inst < n_instancias; inst++){
        Instancia *d = &instancias[inst];
        int *ordem_prioridade = (int *)malloc(sizeof(int) * d->n_tipos);
        int p;

        //Prioridade automatica: ordem em que os tipos foram declarados (so pelo fato de estar iniciando automaticamente)
        for (p = 0; p < d->n_tipos; p++)
            ordem_prioridade[p] = p + 1;

        //ordenar_por_area_desc(d->tipos, d->n_tipos);

        printf("==========================================\n");
        printf(" Instancia: %s | Chapa: %dx%d cm | %d tipos\n", d->nome, d->chapa_larg, d->chapa_alt, d->n_tipos);
        printf("==========================================\n");

        d->area_chapa = d->chapa_larg * d->chapa_alt;

        //Mede APENAS o tempo interno do algoritmo guloso 
        clock_t inicio = clock();
        Resultado res = executar_algoritmo_guloso(d->chapa_larg, d->chapa_alt, d->tipos, d->n_tipos, ordem_prioridade);
        clock_t fim = clock();
        d->tempo_nosso = (double)(fim - inicio) / CLOCKS_PER_SEC;

        //Salva resultados na struct para a tabela final
        d->cortes_nosso   = res.quantidade_cortes;
        d->area_utilizada = res.area_utilizada;
        d->area_sobra     = res.area_sobra;
        d->sobra_nossa    = (res.area_sobra * 100.0) / d->area_chapa;

        exibir_resultado_instancia(d, res, ordem_prioridade);

        liberar_resultado(&res);
        free(ordem_prioridade);
    }

    exibir_tabela_comparativa(instancias, n_instancias);

    return 0;
}

/*int cmp_area_desc(const void *a, const void *b)
{
    int area_a = ((TipoCorte *)a)->largura * ((TipoCorte *)a)->altura;
    int area_b = ((TipoCorte *)b)->largura * ((TipoCorte *)b)->altura;
    return area_b - area_a; 
}

void ordenar_por_area_desc(TipoCorte *tipos, int n_tipos)
{
    qsort(tipos, n_tipos, sizeof(TipoCorte), cmp_area_desc);
}*/


//Retorna 1 se houver sobreposicao, 0 se estiver livre 
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

//Retorna 1 se o retangulo couber dentro da chapa 
int cabe_na_chapa(Retangulo *novo, int chapa_larg, int chapa_alt)
{
    return (novo->x >= 0) &&
           (novo->y >= 0) &&
           (novo->x + novo->largura <= chapa_larg) &&
           (novo->y + novo->altura  <= chapa_alt);
}

/*
 * Gera dois novos cantos candidatos apos posicionar um corte:
     - (x, y + altura)       -> canto acima
     - (x + largura, y)      -> canto a direita
 */
void adicionar_cantos(Canto *cantos, int *n_cantos, int max_cantos, Retangulo *corte)
{
    int cx1 = corte->x, cy1 = corte->y + corte->altura;
    int cx2 = corte->x + corte->largura, cy2 = corte->y;

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

/* -----------------------------------------------------------------------
 * HEURISTICA GULOSA: GRAU DE OCUPACAO
 * ----------------------------------------------------------------------- */

/*
   Conta quantas bordas candidatas estao encostadas em paredes ou outros cortes
   quando o valor for maior o encaixe vai ser mais justo e ai a decisao gulosa e melhor
 */
double calcular_grau_ocupacao(Canto c, int larg, int alt,
                               Retangulo *cortes, int n_cortes,
                               int chapa_larg, int chapa_alt)
{
    int bordas = 0, i;

    if(c.x == 0) bordas++;
    if(c.y == 0) bordas++;
    if(c.x + larg == chapa_larg) bordas++;
    if(c.y + alt  == chapa_alt) bordas++;

    for(i = 0; i < n_cortes; i++){
        if((c.x + larg == cortes[i].x) && !(c.y >= cortes[i].y + cortes[i].altura || c.y + alt <= cortes[i].y))
            bordas++;
        if((c.x == cortes[i].x + cortes[i].largura) && !(c.y >= cortes[i].y + cortes[i].altura || c.y + alt <= cortes[i].y))
            bordas++;
        if((c.y + alt == cortes[i].y) && !(c.x >= cortes[i].x + cortes[i].largura || c.x + larg <= cortes[i].x))
            bordas++;
        if((c.y == cortes[i].y + cortes[i].altura) && !(c.x >= cortes[i].x + cortes[i].largura || c.x + larg <= cortes[i].x))
            bordas++;
    }

    return (double)bordas;
}

/*
   Para cada canto disponivel, testa os tipos na ordem de prioridade
   Escolhe a combinacao (canto + tipo) com maior grau de ocupacao
 */
Resultado executar_algoritmo_guloso(int chapa_larg, int chapa_alt, TipoCorte *tipos, int n_tipos, int *ordem_prioridade)
{
    Resultado res;
    int n_cantos = 0, i, p;

    //Calcula limites de alocacao dinamica 
    int max_cortes = 0, max_cantos_base = 0;
    for(i = 0; i < n_tipos; i++){
        int pot = (chapa_larg / tipos[i].largura) * (chapa_alt / tipos[i].altura);

        if(tipos[i].max_quantidade > 0)
            max_cortes += tipos[i].max_quantidade;
        else
            max_cortes += pot;

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
        free(cantos); free(res.posicoes); free(res.contagem_por_tipo);
        res.quantidade_cortes = 0; res.area_utilizada = 0;
        res.area_sobra = chapa_larg * chapa_alt;
        return res;
    }

    for(i = 0; i < n_tipos; i++)
        res.contagem_por_tipo[i] = 0;

    cantos[0].x = 0; cantos[0].y = 0; n_cantos = 1;

    while(n_cantos > 0){
        int melhor_canto_idx = -1;
        int melhor_tipo_idx  = -1;
        double melhor_grau = -1.0;

        for(i = 0; i < n_cantos; i++){
            for(p = 0; p < n_tipos; p++){
                int t = ordem_prioridade[p] - 1;

                //Verifica o limite maximo de cada tipo 
                if (tipos[t].max_quantidade > 0 && res.contagem_por_tipo[t] >= tipos[t].max_quantidade)
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

                //Substitui com grau maior 
                if(grau > melhor_grau){
                    melhor_grau = grau;
                    melhor_canto_idx = i;
                    melhor_tipo_idx = p;
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
/*
Resultado executar_algoritmo_guloso(int chapa_larg, int chapa_alt, TipoCorte *tipos, int n_tipos, int *ordem_prioridade)
{
    Resultado res;
    int n_cantos = 0, i, p;
   
    //Calcula limites de alocacao dinamica 
    int max_cortes = 0, max_cantos_base = 0;
    for(i = 0; i < n_tipos; i++){
        int pot = (chapa_larg / tipos[i].largura) * (chapa_alt / tipos[i].altura);

        if(tipos[i].max_quantidade > 0)
            max_cortes += tipos[i].max_quantidade;
        else
            max_cortes += pot;

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
        free(cantos); free(res.posicoes); free(res.contagem_por_tipo);
        res.quantidade_cortes = 0; res.area_utilizada = 0;
        res.area_sobra        = chapa_larg * chapa_alt;
        return res;
    }

    for(i = 0; i < n_tipos; i++)
        res.contagem_por_tipo[i] = 0;

    cantos[0].x = 0; cantos[0].y = 0; n_cantos = 1;

    while (n_cantos > 0){
        int melhor_canto_idx = -1;
        int melhor_tipo_idx = -1;
        double melhor_grau = -1.0;

        for(i = 0; i < n_cantos; i++){
            for (p = 0; p < n_tipos; p++){
                int t = ordem_prioridade[p] - 1;

                //Verifica o limite maximo de cada tipo 
                if(tipos[t].max_quantidade > 0 && res.contagem_por_tipo[t] >= tipos[t].max_quantidade)
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

                //Substitui com grau maior
                if(grau > melhor_grau){
                    melhor_grau = grau;
                    melhor_canto_idx = i;
                    melhor_tipo_idx = p;
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
*/
void exibir_resultado_instancia(Instancia *d, Resultado res, int *ordem_prioridade)
{
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
         if(ordem_prioridade[p] - 1 == i){
            pos_pri = p + 1; 
            break; 
         }
      }    
      printf("  Tipo %d (%dx%d cm, prior.%d): %d peca(s)\n", i + 1, d->tipos[i].largura, d->tipos[i].altura, pos_pri, res.contagem_por_tipo[i]);
    }

    printf(" Area utilizada : %d cm2  (%.2f%%)\n", res.area_utilizada, pct_uso);

    if(res.area_sobra > 0)
        printf(" Sobra : %d cm2  (%.2f%%)\n", res.area_sobra, pct_sobra);
    else
        printf("  Sobra : Nenhuma (aproveitamento total!)\n");

    printf("  Tempo algoritmo: %.6f segundos\n", d->tempo_nosso);
    printf("------------------------------------------\n\n");
}

void exibir_tabela_comparativa(Instancia *inst, int n)
{
    int i;

    printf("\n");
    printf("+-------+----------+--------+--------+-----------+-----------+----------+----------+--------------+--------------+\n");
    printf("| %-5s | %-8s | %-6s | %-6s | %-9s | %-9s | %-8s | %-8s | %-12s | %-12s |\n",
           "Inst.", "Chapa", "Pecas", "Nosso", "Area cm2",
           "Usada cm2", "Sobra%Art", "Sobra%Nos",
           "Tempo Art(s)", "Tempo Nos(s)");
    printf("+-------+----------+--------+--------+-----------+-----------+----------+----------+--------------+--------------+\n");

    for(i = 0; i < n; i++){
        Instancia *d = &inst[i];
        char chapa_str[16];
        sprintf(chapa_str, "%dx%d", d->chapa_larg, d->chapa_alt);

        printf("| %-5s | %-8s | %-6d | %-6d | %-9d | %-9d | %-8.2f | %-8.2f | %-12.2f | %-12.6f |\n", d->nome, chapa_str, d->n_pecas_artigo, d->cortes_nosso, d->area_chapa, d->area_utilizada, d->sobra_artigo, d->sobra_nossa, d->tempo_artigo, d->tempo_nosso);
    }

    printf("+-------+----------+--------+--------+-----------+-----------+----------+----------+--------------+--------------+\n");

    printf("\nLegenda:\n");
    printf("  Inst.        : nome da instancia do artigo\n");
    printf("  Chapa        : dimensoes da chapa (LxA em cm)\n");
    printf("  Pecas        : numero de tipos de corte definidos na instancia\n");
    printf("  Nosso        : cortes efetivamente realizados pelo nosso algoritmo\n");
    printf("  Area cm2     : area total da chapa\n");
    printf("  Usada cm2    : area coberta pelos cortes\n");
    printf("  Sobra%%Art   : %% sobra segundo o artigo (Pentium 4, 2.0GHz)\n");
    printf("  Sobra%%Nos   : %% sobra do nosso algoritmo\n");
    printf("  Tempo Art(s) : tempo CPU relatado no artigo\n");
    printf("  Tempo Nos(s) : tempo CPU medido APENAS no algoritmo guloso\n");

    printf("\nAvaliacao:\n");
    for(i = 0; i < n; i++){
        Instancia *d = &inst[i];
        printf("  %s -> Sobra: %s (%.2f%% nosso vs %.2f%% artigo) | Tempo: %s (%.6fs nosso vs %.2fs artigo)\n", d->nome, d->sobra_nossa <= d->sobra_artigo ? "IGUAL/MELHOR" : "PIOR", d->sobra_nossa, d->sobra_artigo, d->tempo_nosso <= d->tempo_artigo ? "MAIS RAPIDO"  : "MAIS LENTO", d->tempo_nosso, d->tempo_artigo);
    }
    printf("\n");
}

void liberar_resultado(Resultado *res)
{
    if (res->posicoes)          { free(res->posicoes);          res->posicoes          = NULL; }
    if (res->contagem_por_tipo) { free(res->contagem_por_tipo); res->contagem_por_tipo = NULL; }
}
