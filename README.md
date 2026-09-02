# ALGORITMO GULOSO COM ESTRATÉGIA DE OCUPAÇÃO DE CANTOS, BUSCAS LOCAIS E GRASP PARA MINIMIZAÇÃO DE SOBRAS NO CORTE DE CHAPAS INDUSTRIAIS.

Trabalho desenvolvido para a disciplina de Análise e Projeto de Algoritmos —
Engenharia de Computação, UTFPR - Campus Pato Branco.

## Descrição
Implementação e comparação de quatro abordagens heurísticas para o Problema de
Corte de Retângulos (Rectangle Packing Problem) aplicado ao corte de chapas
metálicas, com o objetivo de minimizar a área desperdiçada (trim loss).

## Algoritmos Implementados
- **CorteChapaAlgGuloso.c** — Algoritmo guloso baseado na estratégia de Ocupação de
  Cantos (Corner-Occupying Action), conforme Chen e Huang (2007).
- **CorteChapa_BL_PermutacaoDePrioridade.c** — Extensão por busca local com permutação
  da ordem de prioridade dos tipos de peças.
- **CorteChapa_BL_Rotacao.c** — Extensão por busca local com rotação de
  peças não quadradas.
- **CorteChapa_MH_GRASP.c** — Metaheurística GRASP (Greedy Randomized Adaptive
  Search Procedure), combinando construção gulosa randomizada com as duas
  buscas locais acima.

## Estrutura do Repositório

```
├── CorteChapaAlgGuloso.c
├── CorteChapa_BL_PermutacaoDePrioridade.c
├── CorteChapa_BL_Rotacao.c
├── CorteChapa_MH_GRASP.c
├── LerCortes.py
├── instancias.txt
└── prompt_instancias.txt
```

## Como Compilar e Executar
**Compilar:**
```bash
gcc CorteChapaAlgGuloso.c -o CorteChapaAlgGuloso.exe
gcc CorteChapa_BL_PermutacaoDePrioridade.c -o CorteChapa_BL_PermutacaoDePrioridade.exe
gcc CorteChapa_BL_Rotacao.c -o CorteChapa_BL_Rotacao.exe
gcc CorteChapa_MH_GRASP.c -o CorteChapa_MH_GRASP.exe
```
**Executar:**
```bash
./CorteChapaAlgGuloso
./CorteChapa_BL_PermutacaoDePrioridade
./CorteChapa_BL_Rotacao
./CorteChapa_MH_GRASP [max_iteracoes] [alfa] [seed]
```
Os algoritmos leem automaticamente o arquivo `instancias.txt` e geram o arquivo
`cortes_saida.txt` com os resultados.

O GRASP aceita parâmetros opcionais de linha de comando:
`max_iteracoes` (padrão: 10), `alfa` (padrão: 0,30) e `seed` (padrão: 12345).

## Visualização dos Resultados
Após a execução, utilize o script Python para gerar os layouts graficamente:
```bash
python LerCortes.py 
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
