# Validação — Amber Arcade 1.0

Verificações realizadas em 26/10/2026.

## Executado

- GCC 13.3, C++17, `-O2 -Wall -Wextra -Wpedantic`: compilação concluída sem avisos.
- 37 verificações automatizadas aprovadas. Cobrem início, movimentação, salto, limites da arena, tiros contínuos, mira automática, dano, invulnerabilidade, esquiva, recargas, projéteis hostis, pulso, coleta, combo, ondas pendentes, melhorias, pausa, som, chefe, vitória, recorde, reinício e telas.
- Partida automatizada completa: seis ondas vencidas, 56 inimigos eliminados, quatro pontos de vida restantes e 50.875 pontos, em aproximadamente 57,47 segundos de simulação. Esse tempo corresponde ao roteiro automatizado e não estima a duração de uma primeira partida humana.
- CMake 3.31.6: configuração e build Release em Linux concluídos; CTest aprovado.
- AddressSanitizer e UndefinedBehaviorSanitizer: as 37 verificações passaram sem acesso inválido ou comportamento indefinido reportado. A detecção de vazamentos ficou desativada devido à restrição de leitura de processos deste ambiente.
- Compilação cruzada com Zig 0.14.1 / Clang para Windows x86-64 concluída sem avisos.
- As cenas de jogo, título, chefe e melhorias foram renderizadas pelo código e convertidas para PNG. Inspeção visual realizada nas quatro cenas.
- Demonstração de oito segundos renderizada a partir de uma simulação com comandos automatizados, usando o mesmo núcleo e renderizador; codificada em MP4.

## Limites

Não foi possível executar a janela Win32 neste ambiente Linux. Permanecem pendentes a conferência do teclado e mouse, áudio, troca de foco, tela cheia e persistência no perfil de um computador Windows. A renderização mantém o buffer fora da tela introduzido na revisão do Bosque de Âmbar; a confirmação visual no Windows para este novo jogo continua pendente.

A inspeção do executável confirma o formato PE32+ de 64 bits e subsistema Windows GUI. O binário usa bibliotecas de sistema do Windows e Universal CRT; não requer DLL externa de SFML/SDL/GCC/libstdc++/winpthread.

O workflow de GitHub Actions foi preparado, sem execução remota observada. O executável não tem assinatura digital nem instalador.
