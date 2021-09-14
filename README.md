# CNAM TP2 : Ecriture d'un Stack Overflow

Le but de ce TP est de se familiariser avec l'exploitation des corruptions mémoire, via l'exploitation d'un stack overflow simple.

En passant, nous allons apprendre à utiliser git, docker, et un debugger : gdb.

## Prérequis

Utiliser la commande "git clone" pour cloner le repertoire git de ce TP sur cette machine.

	jonathan@blackbox:~$ mkdir tp
	jonathan@blackbox:~$ cd tp
	jonathan@blackbox:~/tp$ git clone git@github.com:endrazine/cnam-tp2-sec108.git
	Cloning into 'cnam-tp2-sec108'...
	remote: Enumerating objects: 15, done.
	remote: Counting objects: 100% (15/15), done.
	remote: Compressing objects: 100% (13/13), done.
	remote: Total 15 (delta 1), reused 12 (delta 1), pack-reused 0
	Receiving objects: 100% (15/15), 24.13 KiB | 4.83 MiB/s, done.
	Resolving deltas: 100% (1/1), done.
	jonathan@blackbox:~/tp$ ls
	cnam-tp2-sec108
	jonathan@blackbox:~/tp$ 

Vérifier que vous avez bien telechargé un fichier gdbinit ce faisant.

## Exploitation d'un stack overflow.

Nous allons utiliser le système de virtualisation docker afin de travailler sur les mêmes binaires compilés, et sur le même environnement.

### Installer Docker sur votre machine

Suivre les instructions de Docker: https://docs.docker.com/engine/install/

	apt-get install -y \
	    apt-transport-https \
	    ca-certificates \
	    curl \
	    gnupg-agent \
	    software-properties-common
	    
	curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add -

	add-apt-repository \
	   "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
	   $(lsb_release -cs) \
	   stable"
	   
	apt-get update

	apt-get install -y docker-ce docker-ce-cli containerd.io

	usermod -aG docker ubuntu


### Telecharger la machine docker cible

Utiliser la commande "docker login" pour s'identifier aupres du registry container d'OVH. Voir https://docs.docker.com/engine/reference/commandline/login/

	jonathan@blackbox:~$ docker login 59b7c723.gra7.container-registry.ovh.net -u cnam -p XXXXXXXXXX
	WARNING! Using --password via the CLI is insecure. Use --password-stdin.
	WARNING! Your password will be stored unencrypted in /home/jonathan/.docker/config.json.
	Configure a credential helper to remove this warning. See
	https://docs.docker.com/engine/reference/commandline/login/#credentials-store

	Login Succeeded
	jonathan@blackbox:~$ 

Les logins et mots de passe seront donnés en cours.

Utiliser la commande "docker pull" pour telecharger l'image docker. Voir https://docs.docker.com/engine/reference/commandline/pull/

	jonathan@blackbox:~$ docker pull 59b7c723.gra7.container-registry.ovh.net/cnamtp/cnamtp:latest
	latest: Pulling from cnamtp/cnamtp
	35807b77a593: Already exists 
	a1b1132ae316: Pull complete 
	Digest: sha256:7707d94e423c84ae76011a2fda0d8ecf005726c5de6ba751bf59b41bd796f910
	Status: Downloaded newer image for 59b7c723.gra7.container-registry.ovh.net/cnamtp/cnamtp:latest
	59b7c723.gra7.container-registry.ovh.net/cnamtp/cnamtp:latest
	jonathan@blackbox:~$ 

Tagger le container docker, le renommer cnamtp:latest:

	jonathan@blackbox:~$ docker tag 59b7c723.gra7.container-registry.ovh.net/cnamtp/cnamtp:latest cnamtp:latest
	jonathan@blackbox:~$ 

Utiliser la commande "docker images" pour lister les images telechargées. Quelle est la taille de l'image telechargée ? (Voir https://docs.docker.com/engine/reference/commandline/images/)

	jonathan@blackbox:~$ docker images
	REPOSITORY                                               TAG                   IMAGE ID       CREATED          SIZE
	cnamtp                                                   latest                b60f468601ea   14 minutes ago   492MB
	59b7c723.gra7.container-registry.ovh.net/cnamtp/cnamtp   latest                b60f468601ea   14 minutes ago   492MB
	jonathan@blackbox:~$ 

### Lancer l'image docker et se familiariser avec docker

Utiliser la commande "docker run" pour lancer l'image. Voir https://docs.docker.com/engine/reference/commandline/run/

	docker run -it --entrypoint /bin/bash cnamtp

Vérifier que l'image est bian lancée au moyen de la commande "docker ps". Voir https://docs.docker.com/engine/reference/commandline/ps/

	jonathan@blackbox:~$ docker ps
	CONTAINER ID   IMAGE     COMMAND       CREATED          STATUS          PORTS     NAMES
	4d4d598da900   cnamtp    "/bin/bash"   46 seconds ago   Up 42 seconds             naughty_bhaskara
	jonathan@blackbox:~$ 


#### Attention: Si vous travaillez dans docker, toute modification est perdue si vous ne sauvegardez pas les résultats au moyen de la commande "docker commit". https://docs.docker.com/engine/reference/commandline/commit/


Dans un second terminal, utiliser la commande "docker exec" pour obtenir un shell dans l'environement docker. Voir https://docs.docker.com/engine/reference/commandline/exec/

	jonathan@blackbox:~$ docker ps
	CONTAINER ID   IMAGE     COMMAND       CREATED          STATUS          PORTS     NAMES
	4d4d598da900   cnamtp    "/bin/bash"   46 seconds ago   Up 42 seconds             naughty_bhaskara
	jonathan@blackbox:~$ docker exec -it 4d4d598da900 /bin/bash
	root@4d4d598da900:/# 


Installer build-essential et gdb dans l'environement docker.

	jonathan@blackbox:~$ docker exec -it 4d4d598da900 /bin/bash
	root@4d4d598da900:/# apt update
	Get:1 http://archive.ubuntu.com/ubuntu focal InRelease [265 kB]
	Get:2 http://security.ubuntu.com/ubuntu focal-security InRelease [114 kB]           
	Get:3 http://archive.ubuntu.com/ubuntu focal-updates InRelease [114 kB]
	Get:4 http://archive.ubuntu.com/ubuntu focal-backports InRelease [101 kB]
	(...)
	root@4d4d598da900:/# apt install build-essential
	Reading package lists... Done
	Building dependency tree       
	Reading state information... Done
	The following additional packages will be installed:
	(...)
	0 upgraded, 84 newly installed, 0 to remove and 0 not upgraded.
	Need to get 57.4 MB of archives.
	After this operation, 256 MB of additional disk space will be used.
	Do you want to continue? [Y/n] Y
	(...)
	Setting up gnupg (2.2.19-3ubuntu2.1) ...
	Processing triggers for libc-bin (2.31-0ubuntu9.2) ...
	root@4d4d598da900:/# 


	root@4d4d598da900:/# apt install gdb
	Reading package lists... Done
	Building dependency tree       
	Reading state information... Done
	The following additional packages will be installed:
	(...)
	Need to get 30.1 MB of archives.
	After this operation, 161 MB of additional disk space will be used.
	Do you want to continue? [Y/n] 
	(...)
	  1. Africa  2. America  3. Antarctica  4. Australia  5. Arctic  6. Asia  7. Atlantic  8. Europe  9. Indian  10. Pacific  11. SystemV  12. US  13. Etc
	Geographic area: 8

	Please select the city or region corresponding to your time zone.

	  1. Amsterdam  6. Belgrade    11. Budapest    16. Gibraltar    21. Jersey       26. Ljubljana   31. Mariehamn  36. Oslo       41. Rome        46. Simferopol  51. Tirane     56. Vatican    61. Zagreb
	  2. Andorra    7. Berlin      12. Busingen    17. Guernsey     22. Kaliningrad  27. London      32. Minsk      37. Paris      42. Samara      47. Skopje      52. Tiraspol   57. Vienna     62. Zaporozhye
	  3. Astrakhan  8. Bratislava  13. Chisinau    18. Helsinki     23. Kiev         28. Luxembourg  33. Monaco     38. Podgorica  43. San_Marino  48. Sofia       53. Ulyanovsk  58. Vilnius    63. Zurich
	  4. Athens     9. Brussels    14. Copenhagen  19. Isle_of_Man  24. Kirov        29. Madrid      34. Moscow     39. Prague     44. Sarajevo    49. Stockholm   54. Uzhgorod   59. Volgograd
	  5. Belfast    10. Bucharest  15. Dublin      20. Istanbul     25. Lisbon       30. Malta       35. Nicosia    40. Riga       45. Saratov     50. Tallinn     55. Vaduz      60. Warsaw
	Time zone: 37


	Current default time zone: 'Europe/Paris'
	Local time is now:      Mon Sep 13 22:46:50 CEST 2021.
	Universal Time is now:  Mon Sep 13 20:46:50 UTC 2021.
	(...)
	Setting up shared-mime-info (1.15-1) ...
	Processing triggers for libc-bin (2.31-0ubuntu9.2) ...
	root@4d4d598da900:/# 



Utiliser la commande "exit" pour terminer la session.

Sauvegarder l'environement via la commande "docker commit" dont vous avez le la documentation ci dessus. Pour un exemple, voir : https://phoenixnap.com/kb/how-to-commit-changes-to-docker-image

	root@4d4d598da900:/# exit
	exit
	jonathan@blackbox:~$ docker commit 4d4d598da900 cnamtp:latest
	sha256:916e13640d6e67df835229a7537b521af59c6c5770ed475c6b89f492e580ea49
	jonathan@blackbox:~$ 


Relancer une session docker au moyen de la commande "docker exec". Gdb et gcc sont ils toujours bien installés ?

	jonathan@blackbox:~$ docker exec -it 4d4d598da900 /bin/bash
	root@4d4d598da900:/# 

### Désactivation des mesures de sécurité ASLR

Pour faciliter l'exploitation du programme vulnérable, il vous faut désactiver l'ASLR sur la machine hote.

Voir comment faire ici: https://askubuntu.com/questions/318315/how-can-i-temporarily-disable-aslr-address-space-layout-randomization

	jonathan@blackbox:~$ echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
	[sudo] password for jonathan: 
	0
	jonathan@blackbox:~$ cat /proc/sys/kernel/randomize_va_space
	0
	jonathan@blackbox:~$ 


### Copier le fichier gdbinit dans votre environement docker

Le fichier gdbinit présent dans ce repertoire git permet d'obtenir une lecture plus simple dans gdb. Vous devez le copier (dans l'environement docker !) sous ~/.gdbinit et eventuellement /root/.gdbinit (attention au "." devant le nom de ficher). Vous pouvez sauver l'image docker via la commande "docker commit", comme vu précédemment.


### Installer gcc-multilib

	root@41d4b0b83655:~# apt install gcc-multilib

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
	(...)
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

	jonathan@blackbox:~/CNAM/bo/poc$ ls
	Makefile  bo  poc  poc.c
	jonathan@blackbox:~/CNAM/bo/poc$ gdb ./poc 
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
	Reading symbols from ./poc...
	(No debugging symbols found in ./poc)
	gdb$ r
	Starting program: /home/jonathan/CNAM/bo/poc/poc 
	process 31682 is executing new program: /home/jonathan/CNAM/bo/poc/bo
	Welcome: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBB�������� !

	Program received signal SIGSEGV, Segmentation fault.
	--------------------------------------------------------------------------[regs]
	  EAX: 0x00000000  EBX: 0x41414141  ECX: 0x00000000  EDX: 0x0804A016  o d I t S z a p c 
	  ESI: 0xF7F8D000  EDI: 0xF7F8D000  EBP: 0x41414141  ESP: 0xFFFFDC30  EIP: 0x42424242
	  CS: 0023  DS: 002B  ES: 002B  FS: 0000  GS: 0063  SS: 002BError while running hook_stop:
	Cannot access memory at address 0x42424242
	0x42424242 in ?? ()
	gdb$ 

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

	jonathan@blackbox:~/CNAM/bo/exploit$ ./exploit 
	Welcome: 1�Ph//shh/bin��PTSP��!�t�;��
		                             �RS��̀������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������ ����������� !
	$ 


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












