# CNAM TP2 : Ecriture d'un Stack Overflow

Le but de ce TP est de se familiariser avec l'exploitation des corruptions mémoire, via l'exploitation d'un stack overflow simple.

En passant, nous allons apprendre à utiliser git, docker, et un debugger : gdb.

## Prérequis

Utiliser la commande "git clone" pour cloner le repertoire git de ce TP sur cette machine.

Vérifier que vous avez bien telechargé un fichier gdbinit ce faisant.

## Exploitation d'un stack overflow.

Nous allons utiliser le système de virtualisation docker afin de travailler sur les mêmes binaires compilés, et sur le même environnement.

### Installer Docker sur votre machine

Suivre les instructions de Docker: https://docs.docker.com/engine/install/

### Telecharger la machine docker cible

Utiliser la commande "docker login" pour s'identifier aupres du registry container d'OVH. Voir https://docs.docker.com/engine/reference/commandline/login/

Les logins et mots de passe seront donnés en cours.

Utiliser la commande "docker pull" pour telecharger l'image docker. Voir https://docs.docker.com/engine/reference/commandline/pull/

Utiliser la commande "docker images" pour lister les images telechargées. Quelle est la taille de l'image telechargée ? (Voir https://docs.docker.com/engine/reference/commandline/images/)

### Lancer l'image docker et se familiariser avec docker

Utiliser la commande "docker run" pour lancer l'image. Voir https://docs.docker.com/engine/reference/commandline/run/

Vérifier que l'image est bian lancée au moyen de la commande "docker ps". Voir https://docs.docker.com/engine/reference/commandline/ps/

#### Attention: Si vous travaillez dans docker, toute modification est perdue si vous ne sauvegardez pas les résultats au moyen de la commande "docker commit". https://docs.docker.com/engine/reference/commandline/commit/


Dans un second terminal, utiliser la commande "docker exec" pour obtenir un shell dans l'environement docker. Voir https://docs.docker.com/engine/reference/commandline/exec/

Installer build-essential et gdb dans l'environement docker.

Utiliser la commande "exit" pour terminer la session.

Sauvegarder l'environement via la commande "docker commit" dont vous avez le la documentation ci dessus. Pour un exemple, voir : https://phoenixnap.com/kb/how-to-commit-changes-to-docker-image

Relancer une session docker au moyen de la commande "docker exec". Gdb et gcc sont ils toujours bien installés ?

### Désactivation des mesures de sécurité ASLR

Pour faciliter l'exploitation du programme vulnérable, il vous faut désactiver l'ASLR sur la machine hote.

Voir comment faire ici: https://askubuntu.com/questions/318315/how-can-i-temporarily-disable-aslr-address-space-layout-randomization

### Copier le fichier gdbinit dans votre environement docker

Le fichier gdbinit présent dans ce repertoire git permet d'obtenir une lecture plus simple dans gdb. Vous devez le copier (dans l'environement docker !) sous ~/.gdbinit et eventuellement /root/.gdbinit (attention au "." devant le nom de ficher). Vous pouvez sauver l'image docker via la commande "docker commit", comme vu précédemment.

### Exploitation

Nous allons réaliser l'ecriture d'un exploit, etape par etape.

#### Ecriture d'un "trigger"

On cherche à écrire un programme minimal poc.c qui va déclencher la vulnérabilité.

Comme vu en cours, ceci peut se faire au moyen d'un "one liner" écrit en perl.

Ecrire un "one liner" perl qui déclenche la vulnérabilité. Quel est la taille minimal du payload à passer a l'application pour déclencher la vulnérabilité ?

#### Analyse dans le debugger gdb

Lancer le programme vulnérable dans gdb et lui donner un parametre:


	jonathan@blackbox:~/CNAM/bo$ gdb ./bo 
	GNU gdb (Ubuntu 9.2-0ubuntu1~20.04) 9.2
	Copyright (C) 2020 Free Software Foundation, Inc.
	License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
	This is free software: you are free to change and redistribute it.
	There is NO WARRANTY, to the extent permitted by law.
	Type "show copying" and "show warranty" for details.
	This GDB was configured as "x86_64-linux-gnu".
	Type "show configuration" for configuration details.
	For bug reporting instructions, please see:
	<http://www.gnu.org/software/gdb/bugs/>.
	Find the GDB manual and other documentation resources online at:
	    <http://www.gnu.org/software/gdb/documentation/>.

	For help, type "help".
	Type "apropos word" to search for commands related to "word"...
	Reading symbols from ./bo...
	(No debugging symbols found in ./bo)
	gdb$ r AAAAA...
	Starting program: /home/jonathan/CNAM/bo/bo AAAAA...
	Welcome: AAAAA... !
	[Inferior 1 (process 431942) exited normally]
	-----------------------------------------------------------------------------------------------------------------------[regs]
	  RAX: 0xError while running hook_stop:
	Value can't be converted to integer.
	gdb$ 

Note: On utilise la commande "exit" pour sortir de gdb.

Utiliser votre "one liner" pour déclencher la vulnérabilité. Lorsque le programme crash, utiliser la commande "context" dans gdb pour visualiser la valeur des registres.

Modifier votre "one liner" pour que tous les bytes rééscris dans la stack valent 0x41 (equivalent à la lettre "A" en ASCII), sauf la valeur de "saved eip" qui doit être mise à 0x42424242 ("BBBB" en ASCII).


#### Ecriture d'un exploit

Créer un fichier exploit.c et son Makefile.

On souhaite utiliser l'appel systeme execve() dans exploit.c pour lancer le programme vulnérable avec notre payload (pour l'instant, le "one liner" écrit plus haut).

Le programme exploit ne doit pas prendre d'arguments, mais doit lancer le programme vulnérable avec des parametre hardcodés.

##### Controle de la valeur de retour

Verifier qu'en lancant le programme exploit, on execute bien le programme vulnérable, avec les arguments souhaités.

Lancer exploit dans gdb. Constater que l'on controle bien eip, la valeur de la prochaine instruction executée.

##### Pseudo-shellcode

Modifier votre exploit de manière à ce que le shellcode (tous les "A") soient remplacés par une instruction générant un SIGTRAP (utiliser l'opcode 0xCC au lieu de 0x41).

##### Faire pointer le saved eip vers votre pseudo-shellcode

Modifier votre exploit, afin que l'adresse de retour ("BBBB", soit 0x42424242) pour qu'il pointe vers le début du pseudo-shellcode.

Executer exploit dans gdb. Verifier que le shellcode est bien executé (le programme doit se terminer via un signal SIGTRAP à l'execution du premier opcode 0xCC, au lieu de se terminer par un SIGSEGV, c'est  dire une erreur de segmentation).

Attention: Les microprocesseurs Intel sont des processeurs "Little Endian". L'ordre des bytes n'est pas naturel...

##### Shellcode

Modifier votre programme en remplaçant le pseudo-shellcode précédent par un "vrai" shellcode: utliser celui vu en cours.

Vous pouvez remplacer les opcodes 0x41 qui restent par des NOPs (opcode 0x90).

Vérifier qu'en lançant votre exploit, vous executez bien votre shellcode.

Utiliser strace pour vérifier que l'execution de votre exploit lance bien le shellcode souhaité.


##### BONUS: Remplacer le shellcode par un shellcode "bind port"

Prérequis: Obtenir l'IP du container docker. Vous pouvez utiliser le shell script suivant:

	for container in `docker ps|grep -v IMAGE|awk '{print $1}'`
	do
		echo "$container "|tr -d "\n"
		docker inspect -f '{{range.NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $container
	done

Rechercher sur exploit-db.com un shellcode x86 qui ouvre un port sur la machine cible et y bind un terminal.

Lancer votre exploit.

Utiliser netcat pour vous connecter à l'IP de la machine cible et vérifiez que vous pouvez y entrer des commandes arbitraires.

##### Résultats

Envoyer votre exploit final exploit.c sur mon email du CNAM : jonathan.brossard at lecnam.net

### J'ai fini plus tôt !

Good for you ! S'attacher à réaliser les wargames disponibles ici pour progresser: https://overthewire.org/wargames/












