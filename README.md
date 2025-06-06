# I2C-Display-Loop

## Mon Application

Ce projet est une application embarquée sur le microcontrôleur qui affiche des carrés de couleur sous forme de cycle. Nous avons 4 couleurs : bleu, rouge, jaune et vert. L'écran est bleu, formé d'un carré noir découpé en 4 carrés plus petits. Un cycle de 4 carrés de couleurs différentes s'affichent dans le carré noir. 

A chaque seconde, on écris en mémoire le nouvel index correspondant au carré. Puis à la seconde d'après, on lit l'index en mémoire afin d'afficher la couleur suivante et on écrit le nouvel index en mémoire. 

Lorsqu'on appuie sur le bouton RESET, l'ancienne couleur est récupérée puis on l'affiche avant de continuer le cycle de nouveau. De plus, la couleur actuelle est affiché sur l'écran de manière écrite.


## Utilisation 
- Copier `User/` dans un nouveau projet Keil UVision5.
- Compiler (`Build`).
- Flasher sur la carte via le bouton "Load" (utiliser J-Link).
- Vérifier que l’écran s’initialise (en cas de bug : bouton RESET de la carte).
- Observez l’évolution automatique des couleurs à l’écran.
- Appuyer sur le bouton RESET pour voir l'écriture en mémoire




