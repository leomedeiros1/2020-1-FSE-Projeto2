# Projeto 12
### Fundamento de Sistemas Embarcados 2020/01

Leonardo de Araujo Medeiros - 17/0038891

### Execução
1. Faça clone deste repositório e acesse a pasta baixada;
2. Altere para a pasta referente ao servidor desejado;
3. Faça a compilação do programa usando a chamada `$ make`;
4. Execute o binário gerado `$ make run`.

* Nota: Idealmente o servidor distríbuido deve ser executado antes do central, para que as informações iniciais estejam disponíveis.
* Problemas conhecidos: Quando o alarme é disparado caracteres estranhos aparecenem na tela, que só serão removidos na próxima atualização da tela.

### Detalhes
* Leitura dos sensores realizada aproximadamente a cada `200ms`;
* Realizada comunicação entre o servidor distribuido e central aproximadamente a cada `200ms`, com o envio de apenas 12 bytes.
___
Mais informações em [FSE - Projeto 2](https://gitlab.com/fse_fga/projetos/projeto-2)
