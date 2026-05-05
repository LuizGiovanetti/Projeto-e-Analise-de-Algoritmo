# Projeto-e-Analise-de-Algoritmo
# Algoritmo Guloso com Busca Local para Corte de Chapas Metálicas

Trabalho desenvolvido para a disciplina de Análise e Projeto de Algoritmos —
Engenharia de Computação, UTFPR.

## Descrição

Implementação e comparação de três abordagens heurísticas para o Problema de
Corte de Retângulos (Rectangle Packing Problem) aplicado ao corte de chapas
metálicas, com o objetivo de minimizar a área desperdiçada (trim loss).

## Algoritmos Implementados

- **CorteNovo.c** — Algoritmo guloso baseado na estratégia de Ocupação de
  Cantos (Corner-Occupying Action), conforme Chen e Huang (2007).
- **CorteNovoBuscaLocalExaustiva.c** — Extensão por busca local com permutação
  da ordem de prioridade dos tipos de peças.
- **CorteNovoBuscaLocalRotacao.c** — Extensão por busca local com rotação de
  peças não quadradas.

## Estrutura do Repositório

```
├── CorteNovo.c
├── CorteNovoBuscaLocalExaustiva.c
├── CorteNovoBuscaLocalRotacao.c
├── LerCortes.py
├── instancias.txt
└── prompt_instancias.txt
```

## Como Compilar e Executar

**Compilar:**
```bash
gcc CorteNovo.c -o CorteNovo
gcc CorteNovoBuscaLocalExaustiva.c -o CorteExaustiva
gcc CorteNovoBuscaLocalRotacao.c -o CorteRotacao
```

**Executar:**
```bash
./CorteNovo
./CorteExaustiva
./CorteRotacao
```

Os algoritmos leem automaticamente o arquivo `instancias.txt` e geram o arquivo
`cortes_saida.txt` com os resultados.

## Visualização dos Resultados

Após a execução, utilize o script Python para gerar os layouts graficamente:

```bash
python LerCortes.py cortes_saida.txt
```

Será gerada uma imagem PNG para cada instância processada.

**Requisitos:**
```bash
pip install matplotlib
```

## Instâncias

O arquivo `instancias.txt` contém 16 instâncias no total:

- **C11, C21, C41** — Instâncias originais de Chen e Huang (2007), utilizadas
  para validação comparativa.
- **B01 a B13** — Instâncias benchmark criadas para este trabalho, com chapas
  variando de 150×150 a 6000×6000 cm e entre 15 e 150 tipos de peças.

O prompt utilizado para gerar as instâncias benchmark está disponível em
`prompt_instancias.txt`, permitindo reprodução idêntica com SEED = 2026.

## Referência

Chen, D.; Huang, W. (2007). *Greedy algorithm for rectangle-packing problem*.
Computer Engineering.

## Autor

Luiz Eduardo Albano Giovanetti — giovanetti@alunos.utfpr.edu.br

Orientador: Prof. Marco Antonio De Castro Barbosa — mbarbosa@utfpr.edu.br
