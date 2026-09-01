"""
visualizar_cortes.py
--------------------
Le o arquivo cortes_saida.txt gerado pelo programa C e plota o layout
de cortes de cada instancia usando matplotlib.

- Cortes: retangulos brancos com borda preta
- Sobra : fundo cinza escuro
- Sem numeracao nos cortes

Uso:
    python visualizar_cortes.py                          # le cortes_saida.txt, unidade padrao
    python visualizar_cortes.py outro_arquivo.txt         # le arquivo customizado
    python visualizar_cortes.py cortes_saida.txt mm       # define a unidade de medida (ex: mm, m, px, un)

Saida:
    Um arquivo PNG por instancia: corte_C11.png, corte_B01.png, etc.
    Tambem exibe cada figura na tela.
"""

import sys
import os
import matplotlib.pyplot as plt
import matplotlib.patches as patches

UNIDADE_PADRAO = 'un' # Unidade de medida usada nos rotulos dos eixos.


def ler_arquivo_cortes(caminho):
    instancias = []
    atual = None

    with open(caminho, 'r', encoding='utf-8') as f:
        for linha in f:
            linha = linha.strip()
            if not linha:
                continue

            partes = linha.split()

            if partes[0] == 'INSTANCIA':
                atual = {
                    'nome':   partes[1],
                    'larg':   int(partes[2]),
                    'alt':    int(partes[3]),
                    'cortes': []
                }

            elif partes[0] == 'CORTE':
                if atual is not None:
                    atual['cortes'].append((
                        int(partes[1]),   # x  (canto inf-esq)
                        int(partes[2]),   # y
                        int(partes[3]),   # largura
                        int(partes[4])    # altura
                    ))

            elif partes[0] == 'FIM':
                if atual is not None:
                    instancias.append(atual)
                    atual = None

    return instancias


def plotar_instancia(inst, unidade=UNIDADE_PADRAO, salvar=True, mostrar=True):
    nome   = inst['nome']
    larg   = inst['larg']
    alt    = inst['alt']
    cortes = inst['cortes']
    
    sufixo = f' ({unidade})' if unidade else ''

    escala = 6.0
    if larg >= alt:
        fig_w = escala
        fig_h = escala * alt / larg
    else:
        fig_h = escala
        fig_w = escala * larg / alt

    fig_w = max(fig_w, 3.0)
    fig_h = max(fig_h, 3.0)

    fig, ax = plt.subplots(figsize=(fig_w, fig_h))

    # fundo cinza escuro representa a sobra (area nao utilizada)
    fundo = patches.Rectangle(
        (0, 0), larg, alt,
        linewidth=2,
        edgecolor='black',
        facecolor='#4A4A4A',   # cinza escuro = sobra
        zorder=0
    )
    ax.add_patch(fundo)

    # cada corte: branco com borda preta, sem numero
    for (x, y, w, h) in cortes:
        ret = patches.Rectangle(
            (x, y), w, h,
            linewidth=0.8,
            edgecolor='black',
            facecolor='white',  # corte = branco
            zorder=1
        )
        ax.add_patch(ret)

    ax.set_xlim(0, larg)
    ax.set_ylim(0, alt)
    ax.set_aspect('equal')
    ax.invert_yaxis()

    ax.set_xlabel(f'Largura{sufixo}', fontsize=9)
    ax.set_ylabel(f'Altura{sufixo}', fontsize=9)

    area_total = larg * alt
    area_usada = sum(w * h for _, _, w, h in cortes)
    pct_sobra  = (area_total - area_usada) / area_total * 100

    ax.set_title(
        f'Instancia {nome}  |  Chapa {larg}x{alt}{sufixo}  |  '
        f'{len(cortes)} cortes  |  Sobra {pct_sobra:.2f}%',
        fontsize=10, pad=8
    )

    ax.tick_params(labelsize=7)
    fig.tight_layout()

    if salvar:
        nome_arquivo = f'corte_{nome}.png'
        fig.savefig(nome_arquivo, dpi=150, bbox_inches='tight')
        print(f'[OK] Salvo: {nome_arquivo}')

    if mostrar:
        plt.show()

    plt.close(fig)


def main():
    caminho = sys.argv[1] if len(sys.argv) > 1 else 'cortes_saida.txt'
    unidade = sys.argv[2] if len(sys.argv) > 2 else UNIDADE_PADRAO

    if not os.path.isfile(caminho):
        print(f'[ERRO] Arquivo nao encontrado: {caminho}')
        sys.exit(1)

    instancias = ler_arquivo_cortes(caminho)

    if not instancias:
        print('[ERRO] Nenhuma instancia encontrada no arquivo.')
        sys.exit(1)

    print(f'Lidas {len(instancias)} instancia(s) de "{caminho}".')

    for inst in instancias:
        print(f'  Plotando {inst["nome"]} ({inst["larg"]}x{inst["alt"]}, '
              f'{len(inst["cortes"])} cortes)...')
        plotar_instancia(inst, unidade=unidade, salvar=True, mostrar=True)

    print('Concluido.')


if __name__ == '__main__':
    main()
