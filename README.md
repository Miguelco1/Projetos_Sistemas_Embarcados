# Projetos_Sistemas_Embarcados
UTFPR - Universidade Tecnológica Federal do Paraná
Sistemas Embarcados 2021.1 
Professor: Hugo Vieira Neto
Aluno: Miguel Ferreira Chagas Neto


Projeto 1- Conclusões e Observações:
A imprecisão da temporização aumenta conforme o aumento no clock.

Otimizações do compilador:
- Low: No low o compilador não iterferiu de manerias muito comprometedoras no fluxo do programa
apenas transformando o loop while em uma iteração de 3 comandos assembly ao invés de 4, como ocorre 
quando não se tem otimizações. 9 trocas em 10 segundos

- Medium: Em média otimização o programa continua sendo executado de forma parecida com a otimização baixa, continuando com o mesmo tempo de aproximação de 1 segundo para os estados dos leds.

- High: Em alta otimização o compilador exclui o loop de espera, especulo que seja por não alterar nada que o compilador julgue como relevante, do fluxo do programa e se mantém em alterar o valor da variável LED_D4 e escrevela no GPIO, alterando assim pra uma função cujo o tempo de cada estado do led é cerca de 10 ciclos de clock, tornando a troca de estados imperceptiveis a olho nu.

- 120mhz: Em 120mhz o led realizou cerca de 40 trocas em 10 segundos enquanto em 24mhz ele realizou cerca de 9, logo há uma diferença de cerca de 4,4444 vezes entre os tempos. O que indica uma perda de precisão com o aumento do clock e ajuda a reforçar que laços de espera em software não são a melhor alternativa por conta da quantidade de fatores que os podem influenciar.