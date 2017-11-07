_The 5 October 2017_
### Software Engineering Project

## Introduction :    
The Software Engineering Project is a software which purpose is to teach
students how to cooperate as a team in a professional environment. Students
must be prove to the teacher and to the tutors their ability to develop a
finished product within the lines of the Agile methodology. (...)

## Final Product :    
The final product to deliver by the end of the semester is a mathematical
(algebraic) __engine__ able to parse, resolve and manipulate basic first and
second degree equations with one or two unknowns. The purpose of the engine is
to automatically correct students equation solutions. As such the _development_
of the actual student final answer bears as much, if not more, importance than
the final result.

## To do as soon as possible :
  1. We need to understand where the data linked to our algebraic expression
     engine resides within the database. -> Way to store
     Check the model of the database.

  2. Contact (maybe) the customer in order to have user stories that fit the actual demand.

## Deadlines
  1. __6.10.17__
  -> Make a draft version of the project requirements with the intention of  
     being able to meet the customer by the 13.10.17
  -> Usage scenarios - With the customer needs // No ambiguities

  2. __13.10.17__
  -> User stories
  -> UML Schemas

## Sidenote

  Download an Agile development system (Jira / Trello).

  Quality is the important point here. The software must be ready for
  production by the end of the semester.

  Second team leader :





  ## Use cases :

1. __Situation initiale__
  - Oscar se postionne comme un site web pour :
    Ecole élémentaire
    Ecole secondaire
  - Propose un support pour les mathématiques
  - Les compétences sont divisées en catégories
  - Dans le cadre des catégories, un étudiant peut se trouver dans un état de
    + Pas Testé
    + Testé
      + Maitrise
      + Non maitrise
  - Quand un étudiant effectue un test, le status précité est mis à jour, (régrèsse, progrèsse)
  - Différents types de ressources sont mis à disposition pour aider l'étudiant à comprendre la matière ciblée
  - Site web principalement utilisé sur des smartphones et tablettes
    (Site web doit être responsive (différentes diagonales)

2. __Pour développer notre système, nous partons donc de la situation où :__

    La partie de notre projet se concentrant principalement sur les equations,
    nous toucheront principalement les élèves du secondaire.

    Le système peut être accédé via deux status distant, celui de :
      + professeur ou,
      + étudiant.

    __Professeur__ :


    __Etudiant__ :
    Enregistrement effectué par l'étudiant (il semble, avec un matricule).
    (ou par l'établissement) ?

    Un étudiant se connecte à un site web où il s'enregistre à des classes qui
    sont données par un professeur. Depuis cet espace il a la possibilité
    d'accéder à la théorie ou aux exercices.

    En fonction des capacités de l'élève et des cours auquel il est inscrit,
    plusieurs séries d'exercices pour un même cours lui sont disponibles. La
    completion de ces exercices entraine une ré-évaluation du niveau de
    celui-ci dans la matière. Cette completion implique un changement de statut.
