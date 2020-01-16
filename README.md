# Tutorat Système

L'objectif de ce mini-projet est de comprendre le fonctionnement du bus USB d'une machine. Pour ce faire, nous disposons d'une Arduino Uno ainsi que d'un shield de manette de jeu et un mini-PCB de 6 LED.

Le projet se divise en trois parties distinctes :
- Programmer l'ATMega328p pour acquérir l'état des 5 boutons du shield et des LED des lignes 8 à 13
- Programmer l'ATMega16u2 pour gérer le bus USB et lui implanter un périphérique USB
- Écrire un programme C sur la machine pour utiliser la manette

L'idée finale est de commander un "Space Invaders" avec notre manette USB, cependant quelques étapes de la programmation de l'ATMega16u2 n'ont pas permis d'arriver à ce stade du projet.

## Pour commencer

Vous trouverez tous les fichiers et dossiers liés à ce projet sur ce dépôt git. Téléchargez l'ensemble du répertoire ``lukas-fauchois/TutoratSysteme`` pour passer à la suite.

### Pré-requis

Avant de pouvoir exécuter nos programmes il est requis d'installer quelques bibliothèques sur la machine : `libusb-1.0`, `avr`, `avrdude`,`LUFA`.

Une fois l'ensemble des bibliothèques installées, placez vous  dans le dossier extrait et vérifiez si les fichiers suivants s'y trouvent bien :

- ATMega328p/
  - ``main.c``
  - ``Makefile``
- ATMega16u2/PolytechLille/PAD/
  - dans Config/ : ``LUFAConfig.h``
  - ``Descriptor.c`` & ``Descriptor.h``
  - ``asf.xml``, ``doxyfile``, ``Makefile``, ``PAD.c``, ``PAD.h`` & ``PAD.txt``
- USB/
  - ``usb.c``

### Démarrage

Pour pouvoir installer notre programme, il est important de suivre rigoureusement les étapes suivantes, en s'assurant que l'Arduino UNO est bien branchée sur le port `dev/ttyACM0` :

#### Programmer l'ATMega328p

_Etape 1_ : se placer dans le répertoire ATMega328p/<br/>
_Etape 2_ : exécutez la commande ``make upload``<br/>
_Etape 3_ : Pour vérifier la bonne lecture des boutons, joysticks et leds, il est nécessaire d'utiliser la liaison série. Sur la machine, ouvrir un nouveau terminal et entrer la commande ``minicom -8 -o -b 9600 -D /dev/ttyACM0``<br/>

- _Test des leds_ : Pour allumer les leds, utiliser les touches A à F du clavier (qui correspondent chacune à une led). Les entrées en minuscules éteignent les leds correspondantes et inversement en MAJUSCULES.<br/>
- _Test des boutons/joysticks_ : En appuyant sur les boutons ou en modifiant la position du joystick, on reçoit des valeurs par liaison série affichées sur le minicom.

#### Programmer l'ATMega16u2

_Etape 1_ : se placer dans le répertoire ATMega16u2/PolytechLille/PAD/<br/>
_Etape 2_ : exécutez la commande ``make``<br/>
_Etape 3_ : passer l'Arduino en mode `dfu` en court-circuitant les lignes reset et masse présentes sur l'ICSP. Exécutez ensuite la commande ``lsusb | grep DFU`` pour vérifier que l'ATMega16u2 est bien en mode `dfu`<br/>
_Etape 4_ : entrez successivement les commandes ``dfu-programmer atmega16u2 erase``, ``dfu-programmer atmega16u2 flash PAD.hex`` & ``dfu-programmer atmega16u2 reset``<br/>
_Etape 5_ : débranchez puis rebranchez la carte. Exécutez la commande ``lsusb | grep 1234:4321``, si une ligne apparaît à l'exécution de cette commande vous pouvez passer à la suite<br/>

- _Vérification_ : entrez la commande ``lsusb -vvv -d 1234:4321``, elle permet d'afficher les informations concernant le périphérique et nous donne une description de ses interfaces USB. On peut y retrouver les interfaces créées dans les fichiers `PAD` et `Descriptor`, c'est-à-dire une interface IN avec deux endpoints et une interface OUT avec un endpoints, tous de type interruption. On obtient également `idVendor`, `idProduct`, les `bEndpointAdress`, etc.

#### Gestion de la connexion USB avec le 16u2

_Etape 1_ : se placer dans le répertoire USB/<br/>
_Etape 2_ : exécutez la commande ``gcc -o executable_name usb.c -Wall -lusb-1.0``<br/>
_Etape 3_ : exécuter le programme en tapant ``./usb``, on retrouvera normalement les informations affichées dans la phase de _Vérification_ de l'ATMega16u2.<br/>_Remarque_ : les adresses correpondent normalement aux `bEndpointAdress` de l'ATMega16u2.

## Avancée du projet

Les trois parties du projet sont fonctionnelles individuellement. Cependant, nous n'avons pas réussi à lier les trois parties pour rendre le programme complétement opérationnel.

## Fabriqué avec

**VIM** - Editeur de textes

## Auteurs

* **Guillaume Rouillé**
* **Lukas Fauchois**
