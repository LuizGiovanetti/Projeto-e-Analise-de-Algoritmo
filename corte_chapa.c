#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    char nome[32];
    int chapa_larg;
    int chapa_alt;
    int n_pecas;
    double trim_loss;
    double tempo_s;
} ReferenciaArtigo;

static const ReferenciaArtigo REFERENCIAS[] = {
    {"C11",20,20,16,0.00,0.04},
    {"C12",20,20,17,0.00,0.32},
    {"C13",20,20,16,0.00,0.06},
    {"C21",40,15,25,0.00,0.51},
    {"C31",60,30,28,1.11,0.97},
};

static const int N_REF = 5;

typedef struct {
    int x,y,largura,altura,tipo;
} Retangulo;

typedef struct {
    int x,y;
} Canto;

typedef struct {
    int largura;
    int altura;
    int max_quantidade;
} TipoCorte;

typedef struct {
    int quantidade_cortes;
    int area_utilizada;
    int area_sobra;
    Retangulo *posicoes;
    int *contagem_por_tipo;
} Resultado;

typedef struct {
    char nome[32];
    int chapa_larg;
    int chapa_alt;
    int n_tipos;
    TipoCorte tipos[16];
    int prioridade[16];
} Instancia;

int ler_instancias_arquivo(const char *caminho, Instancia *inst, int max_inst);
void ler_instancia_interativo(Instancia *inst);
int verifica_sobreposicao(Retangulo *cortes, int n, Retangulo *novo);
int cabe_na_chapa(Retangulo *novo, int larg, int alt);
void adicionar_cantos(Canto *cantos, int *n, int max, Retangulo *corte);
int canto_ja_existe(Canto *cantos, int n, int x, int y);
double calcular_grau_ocupacao(Canto c,int larg,int alt,Retangulo *cortes,int n_cortes,int chapa_larg,int chapa_alt);
Resultado executar_algoritmo(Instancia *inst);
void exibir_resultado(Resultado res, Instancia *inst);
void exibir_metricas(Resultado res, Instancia *inst, double tempo_s);
void exibir_tabela_comparativa(Resultado *resultados,Instancia *instancias,double *tempos,int n);
void liberar_resultado(Resultado *res);

int main(int argc,char *argv[]){
    if(argc>=2){
        Instancia instancias[16];
        int n=ler_instancias_arquivo(argv[1],instancias,16);
        if(n<=0){
            fprintf(stderr,"[ERRO] Nenhuma instancia valida em: %s\n",argv[1]);
            return 1;
        }

        Resultado resultados[16];
        double tempos[16];

        printf("\n========================================\n");
        printf("  Rodando %d instancia(s) de: %s\n",n,argv[1]);
        printf("========================================\n");

        for(int i=0;i<n;i++){
            printf("\n--- Instancia %s (%dx%d cm, %d tipo(s)) ---\n",
                   instancias[i].nome,
                   instancias[i].chapa_larg,
                   instancias[i].chapa_alt,
                   instancias[i].n_tipos);

            clock_t t0=clock();
            resultados[i]=executar_algoritmo(&instancias[i]);
            clock_t t1=clock();
            tempos[i]=(double)(t1-t0)/CLOCKS_PER_SEC;

            exibir_resultado(resultados[i],&instancias[i]);
            exibir_metricas(resultados[i],&instancias[i],tempos[i]);
        }

        exibir_tabela_comparativa(resultados,instancias,tempos,n);

        for(int i=0;i<n;i++){
            liberar_resultado(&resultados[i]);
        }

    }else{
        Instancia inst;
        ler_instancia_interativo(&inst);

        clock_t t0=clock();
        Resultado res=executar_algoritmo(&inst);
        clock_t t1=clock();

        double tempo=(double)(t1-t0)/CLOCKS_PER_SEC;

        exibir_resultado(res,&inst);
        printf("\nTempo de execucao : %.4f s\n",tempo);
        printf("==========================================\n\n");

        liberar_resultado(&res);
    }
    return 0;
}

int ler_instancias_arquivo(const char *caminho, Instancia *inst, int max_inst){
    FILE *f=fopen(caminho,"r");
    if(!f){
        fprintf(stderr,"[ERRO] Nao foi possivel abrir: %s\n",caminho);
        return 0;
    }

    char linha[256];
    int n=0;

    while(n<max_inst && fgets(linha,sizeof(linha),f)){
        if(linha[0]=='#'||linha[0]=='\n'||linha[0]=='\r') continue;

        if(strncmp(linha,"INSTANCIA",9)==0){
            Instancia *cur=&inst[n];
            memset(cur,0,sizeof(Instancia));
            sscanf(linha,"INSTANCIA %31s",cur->nome);

            do{
                if(!fgets(linha,sizeof(linha),f)) goto fim;
            }while(linha[0]=='#'||linha[0]=='\n'||linha[0]=='\r');
            sscanf(linha,"%d %d",&cur->chapa_larg,&cur->chapa_alt);

            do{
                if(!fgets(linha,sizeof(linha),f)) goto fim;
            }while(linha[0]=='#'||linha[0]=='\n'||linha[0]=='\r');
            sscanf(linha,"%d",&cur->n_tipos);

            for(int i=0;i<cur->n_tipos;i++){
                do{
                    if(!fgets(linha,sizeof(linha),f)) goto fim;
                }while(linha[0]=='#'||linha[0]=='\n'||linha[0]=='\r');

                sscanf(linha,"%d %d %d",
                       &cur->tipos[i].largura,
                       &cur->tipos[i].altura,
                       &cur->tipos[i].max_quantidade);
            }

            do{
                if(!fgets(linha,sizeof(linha),f)) goto fim;
            }while(linha[0]=='#'||linha[0]=='\n'||linha[0]=='\r');

            if(strncmp(linha,"PRIORIDADES",11)==0){
                char *ptr=linha+11;
                for(int i=0;i<cur->n_tipos;i++){
                    while(*ptr==' ') ptr++;
                    cur->prioridade[i]=atoi(ptr);
                    while(*ptr && *ptr!=' ' && *ptr!='\n') ptr++;
                }
            }
            n++;
        }
    }

fim:
    fclose(f);
    return n;
}

void ler_instancia_interativo(Instancia *inst){
    memset(inst,0,sizeof(Instancia));
    strcpy(inst->nome,"manual");

    printf("=== SISTEMA DE CORTE DE CHAPA METALICA ===\n\n");
    printf("Informe as dimensoes da chapa de entrada:\n");

    do{
        printf("  Largura da chapa (cm): ");
        scanf("%d",&inst->chapa_larg);
        if(inst->chapa_larg<=0) printf("  [AVISO] Valor invalido.\n");
    }while(inst->chapa_larg<=0);

    do{
        printf("  Altura  da chapa (cm): ");
        scanf("%d",&inst->chapa_alt);
        if(inst->chapa_alt<=0) printf("  [AVISO] Valor invalido.\n");
    }while(inst->chapa_alt<=0);

    do{
        printf("\nQuantidade de tipos de corte: ");
        scanf("%d",&inst->n_tipos);
        if(inst->n_tipos<1) printf("  [AVISO] Informe ao menos 1.\n");
    }while(inst->n_tipos<1);

    for(int i=0;i<inst->n_tipos;i++){
        printf("\nCorte %d:\n",i+1);
        do{
            do{
                printf("  Largura (cm): ");
                scanf("%d",&inst->tipos[i].largura);
            }while(inst->tipos[i].largura<=0);

            do{
                printf("  Altura  (cm): ");
                scanf("%d",&inst->tipos[i].altura);
            }while(inst->tipos[i].altura<=0);

            if(inst->tipos[i].largura>inst->chapa_larg ||
               inst->tipos[i].altura>inst->chapa_alt)
                printf("  [AVISO] Nao cabe na chapa.\n");

        }while(inst->tipos[i].largura>inst->chapa_larg ||
               inst->tipos[i].altura>inst->chapa_alt);

        do{
            printf("  Max (0=sem limite): ");
            scanf("%d",&inst->tipos[i].max_quantidade);
        }while(inst->tipos[i].max_quantidade<0);
    }

    if(inst->n_tipos>1){
        printf("\nOrdem de prioridade:\n");
        for(int i=0;i<inst->n_tipos;i++){
            do{
                printf("  Prioridade %d (1 a %d): ",i+1,inst->n_tipos);
                scanf("%d",&inst->prioridade[i]);
            }while(inst->prioridade[i]<1 || inst->prioridade[i]>inst->n_tipos);
        }
    }else{
        inst->prioridade[0]=1;
    }
}

int verifica_sobreposicao(Retangulo *cortes,int n,Retangulo *novo){
    for(int i=0;i<n;i++){
        if(!((novo->x+novo->largura)<=cortes[i].x ||
             novo->x>=(cortes[i].x+cortes[i].largura) ||
             (novo->y+novo->altura)<=cortes[i].y ||
             novo->y>=(cortes[i].y+cortes[i].altura)))
            return 1;
    }
    return 0;
}

int cabe_na_chapa(Retangulo *novo,int larg,int alt){
    return novo->x>=0 && novo->y>=0 &&
           novo->x+novo->largura<=larg &&
           novo->y+novo->altura<=alt;
}

void adicionar_cantos(Canto *cantos,int *n,int max,Retangulo *corte){
    int cx1=corte->x;
    int cy1=corte->y+corte->altura;
    int cx2=corte->x+corte->largura;
    int cy2=corte->y;

    if(!canto_ja_existe(cantos,*n,cx1,cy1) && *n<max){
        cantos[*n].x=cx1;
        cantos[*n].y=cy1;
        (*n)++;
    }

    if(!canto_ja_existe(cantos,*n,cx2,cy2) && *n<max){
        cantos[*n].x=cx2;
        cantos[*n].y=cy2;
        (*n)++;
    }
}

int canto_ja_existe(Canto *cantos,int n,int x,int y){
    for(int i=0;i<n;i++){
        if(cantos[i].x==x && cantos[i].y==y) return 1;
    }
    return 0;
}

double calcular_grau_ocupacao(Canto c,int larg,int alt,Retangulo *cortes,int n_cortes,int chapa_larg,int chapa_alt){
    int bordas=0;

    if(c.x==0) bordas++;
    if(c.y==0) bordas++;
    if(c.x+larg==chapa_larg) bordas++;
    if(c.y+alt==chapa_alt) bordas++;

    for(int i=0;i<n_cortes;i++){
        if((c.x+larg==cortes[i].x) &&
           !(c.y>=cortes[i].y+cortes[i].altura || c.y+alt<=cortes[i].y))
            bordas++;

        if((c.x==cortes[i].x+cortes[i].largura) &&
           !(c.y>=cortes[i].y+cortes[i].altura || c.y+alt<=cortes[i].y))
            bordas++;

        if((c.y+alt==cortes[i].y) &&
           !(c.x>=cortes[i].x+cortes[i].largura || c.x+larg<=cortes[i].x))
            bordas++;

        if((c.y==cortes[i].y+cortes[i].altura) &&
           !(c.x>=cortes[i].x+cortes[i].largura || c.x+larg<=cortes[i].x))
            bordas++;
    }
    return (double)bordas;
}

Resultado executar_algoritmo(Instancia *inst){
    int chapa_larg=inst->chapa_larg;
    int chapa_alt=inst->chapa_alt;
    int n_tipos=inst->n_tipos;
    TipoCorte *tipos=inst->tipos;
    int *ordem=inst->prioridade;

    Resultado res;
    int n_cantos=0;

    int max_cortes=0;
    int max_cantos_base=0;

    for(int i=0;i<n_tipos;i++){
        int pot=(chapa_larg/tipos[i].largura)*(chapa_alt/tipos[i].altura);
        max_cortes+=pot;
        if(pot>max_cantos_base) max_cantos_base=pot;
    }

    int max_cantos=2*max_cantos_base+4;

    Canto *cantos=malloc(sizeof(Canto)*max_cantos);
    res.posicoes=malloc(sizeof(Retangulo)*(max_cortes+1));
    res.contagem_por_tipo=malloc(sizeof(int)*n_tipos);

    res.quantidade_cortes=0;
    res.area_utilizada=0;

    if(!cantos || !res.posicoes || !res.contagem_por_tipo){
        printf("[ERRO] Falha ao alocar memoria.\n");
        free(cantos);
        free(res.posicoes);
        free(res.contagem_por_tipo);
        res.area_sobra=chapa_larg*chapa_alt;
        return res;
    }

    for(int i=0;i<n_tipos;i++) res.contagem_por_tipo[i]=0;

    cantos[0].x=0;
    cantos[0].y=0;
    n_cantos=1;

    while(n_cantos>0){
        int melhor_canto=-1;
        int melhor_tipo=-1;
        double melhor_grau=-1.0;

        for(int i=0;i<n_cantos;i++){
            for(int p=0;p<n_tipos;p++){
                int t=ordem[p]-1;

                if(tipos[t].max_quantidade>0 &&
                   res.contagem_por_tipo[t]>=tipos[t].max_quantidade)
                    continue;

                Retangulo cand;
                cand.x=cantos[i].x;
                cand.y=cantos[i].y;
                cand.largura=tipos[t].largura;
                cand.altura=tipos[t].altura;
                cand.tipo=t;

                if(!cabe_na_chapa(&cand,chapa_larg,chapa_alt)) continue;
                if(verifica_sobreposicao(res.posicoes,res.quantidade_cortes,&cand)) continue;

                double grau=calcular_grau_ocupacao(cantos[i],
                                                   tipos[t].largura,tipos[t].altura,
                                                   res.posicoes,res.quantidade_cortes,
                                                   chapa_larg,chapa_alt);

                if(grau>melhor_grau){
                    melhor_grau=grau;
                    melhor_canto=i;
                    melhor_tipo=p;
                }
            }
        }

        if(melhor_canto==-1) break;

        int t=ordem[melhor_tipo]-1;

        Retangulo novo;
        novo.x=cantos[melhor_canto].x;
        novo.y=cantos[melhor_canto].y;
        novo.largura=tipos[t].largura;
        novo.altura=tipos[t].altura;
        novo.tipo=t;

        res.posicoes[res.quantidade_cortes++]=novo;
        res.area_utilizada+=novo.largura*novo.altura;
        res.contagem_por_tipo[t]++;

        cantos[melhor_canto]=cantos[n_cantos-1];
        n_cantos--;

        adicionar_cantos(cantos,&n_cantos,max_cantos,&novo);
    }

    res.area_sobra=(chapa_larg*chapa_alt)-res.area_utilizada;

    free(cantos);
    return res;
}

void exibir_resultado(Resultado res,Instancia *inst){
    int area_chapa=inst->chapa_larg*inst->chapa_alt;
    double pct_uso=res.area_utilizada*100.0/area_chapa;
    double pct_sobra=res.area_sobra*100.0/area_chapa;

    printf("\n  Posicao de cada corte:\n");
    printf("  %-6s %-10s %-8s %-8s\n","Corte","Tipo(LxA)","X(cm)","Y(cm)");
    printf("  %-6s %-10s %-8s %-8s\n","-----","---------","-----","-----");

    for(int i=0;i<res.quantidade_cortes;i++){
        int t=res.posicoes[i].tipo;
        char dim[16];
        sprintf(dim,"%dx%d",inst->tipos[t].largura,inst->tipos[t].altura);

        printf("  %-6d %-10s %-8d %-8d\n",
               i+1,dim,res.posicoes[i].x,res.posicoes[i].y);
    }

    printf("\n  Cortes realizados : %d\n",res.quantidade_cortes);

    for(int i=0;i<inst->n_tipos;i++){
        printf("    Tipo %d (%dx%d): %d peca(s)\n",
               i+1,
               inst->tipos[i].largura,
               inst->tipos[i].altura,
               res.contagem_por_tipo[i]);
    }

    printf("  Area utilizada    : %d cm2 (%.2f%%)\n",res.area_utilizada,pct_uso);
    printf("  Sobra             : %d cm2 (%.2f%%)\n",res.area_sobra,pct_sobra);
}

void exibir_metricas(Resultado res,Instancia *inst,double tempo_s){
    int area_chapa=inst->chapa_larg*inst->chapa_alt;
    double trim_loss=res.area_sobra*100.0/area_chapa;

    const ReferenciaArtigo *ref=NULL;

    for(int k=0;k<N_REF;k++){
        if(strcmp(inst->nome,REFERENCIAS[k].nome)==0){
            ref=&REFERENCIAS[k];
            break;
        }
    }

    printf("\n  Comparacao com artigo (instancia %s):\n",inst->nome);
    printf("  %-30s  %-15s  %-15s\n","Metrica","Artigo(2007)","Este codigo");
    printf("  %-30s  %-15s  %-15s\n","------------------------------","---------------","---------------");

    char buf[32];

    sprintf(buf,"%.2f%%",trim_loss);

    if(ref)
        printf("  %-30s  %-14.2f%%  %-15s\n","Trim loss (area nao usada)",ref->trim_loss,buf);
    else
        printf("  %-30s  %-15s  %-15s\n","Trim loss (area nao usada)","N/A",buf);

    sprintf(buf,"%d",res.quantidade_cortes);

    if(ref)
        printf("  %-30s  %-15d  %-15s\n","Pecas posicionadas",ref->n_pecas,buf);
    else
        printf("  %-30s  %-15s  %-15s\n","Pecas posicionadas","N/A",buf);

    sprintf(buf,"%.4f s",tempo_s);

    if(ref)
        printf("  %-30s  %-14.2f s  %-15s\n","Tempo CPU",ref->tempo_s,buf);
    else
        printf("  %-30s  %-15s  %-15s\n","Tempo CPU","N/A",buf);

    if(ref){
        double diff=trim_loss-ref->trim_loss;

        if(diff<=0.0)
            printf("\n  [OK] Trim loss igual ou melhor que o artigo.\n");
        else if(diff<=1.0)
            printf("\n  [~]  Trim loss %.2f%% acima do artigo (dentro de 1%%).\n",diff);
        else
            printf("\n  [!]  Trim loss %.2f%% acima do artigo.\n",diff);
    }
}

void exibir_tabela_comparativa(Resultado *resultados,Instancia *instancias,double *tempos,int n){
    printf("\n");
    printf("============================================================\n");
    printf("  TABELA GERAL — Chen & Huang (2007) vs. Este Codigo\n");
    printf("============================================================\n");

    printf("  %-6s  %-10s  %8s  %8s  %8s  %9s  %9s\n",
           "Inst.","Container","Art.TL%","Nos.TL%","Diff","Art.t(s)","Nos.t(s)");

    printf("  %-6s  %-10s  %8s  %8s  %8s  %9s  %9s\n",
           "------","----------","-------","-------","-------","---------","---------");

    double soma_tl_art=0;
    double soma_tl_nos=0;
    double soma_t_art=0;
    double soma_t_nos=0;
    int n_ref=0;

    for(int i=0;i<n;i++){
        int area=instancias[i].chapa_larg*instancias[i].chapa_alt;
        double tl_nos=resultados[i].area_sobra*100.0/area;

        const ReferenciaArtigo *ref=NULL;

        for(int k=0;k<N_REF;k++){
            if(strcmp(instancias[i].nome,REFERENCIAS[k].nome)==0){
                ref=&REFERENCIAS[k];
                break;
            }
        }

        char cont[16];
        sprintf(cont,"%dx%d",instancias[i].chapa_larg,instancias[i].chapa_alt);

        if(ref){
            double diff=tl_nos-ref->trim_loss;

            printf("  %-6s  %-10s  %7.2f%%  %7.2f%%  %+7.2f%%  %8.2f s  %8.4f s\n",
                   instancias[i].nome,cont,
                   ref->trim_loss,tl_nos,diff,
                   ref->tempo_s,tempos[i]);

            soma_tl_art+=ref->trim_loss;
            soma_tl_nos+=tl_nos;
            soma_t_art+=ref->tempo_s;
            soma_t_nos+=tempos[i];
            n_ref++;
        }else{
            printf("  %-6s  %-10s  %8s  %7.2f%%  %8s  %9s  %8.4f s\n",
                   instancias[i].nome,cont,
                   "N/A",tl_nos,"N/A","N/A",tempos[i]);

            soma_tl_nos+=tl_nos;
            soma_t_nos+=tempos[i];
        }
    }

    printf("  %-6s  %-10s  %8s  %8s  %8s  %9s  %9s\n",
           "------","----------","-------","-------","-------","---------","---------");

    if(n_ref>0){
        double media_tl_art=soma_tl_art/n_ref;
        double media_tl_nos=soma_tl_nos/n;

        printf("  %-6s  %-10s  %7.2f%%  %7.2f%%  %+7.2f%%  %8.2f s  %8.4f s\n",
               "MEDIA","",
               media_tl_art,media_tl_nos,
               media_tl_nos-media_tl_art,
               soma_t_art/n_ref,
               soma_t_nos/n);
    }

    printf("============================================================\n");
    printf("  TL%%  = Trim Loss (area nao utilizada em %%)\n");
    printf("  Diff = Nos.TL - Art.TL  (negativo = melhor que artigo)\n");
    printf("  Artigo: Pentium 4 IBM R40, 2.0GHz, 256MB RAM.\n");
    printf("  Tempos nao sao diretamente comparaveis (hardware diferente).\n");
    printf("============================================================\n\n");
}

void liberar_resultado(Resultado *res){
    if(res->posicoes){
        free(res->posicoes);
        res->posicoes=NULL;
    }

    if(res->contagem_por_tipo){
        free(res->contagem_por_tipo);
        res->contagem_por_tipo=NULL;
    }
}