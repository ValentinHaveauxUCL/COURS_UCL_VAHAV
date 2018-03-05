# LINGI2172 Databases TP1
_Haveaux Valentin_

## Question 1
List the names of all the countries.
### SQL
```SQL
SELECT name
FROM Country
```
### TUTORIAL D
```Rel
Country {name}
```

## Question 2
List the names of all the moutains that exceed (strictly) 4000 meters in height.
### SQL
```SQL
SELECT name
FROM Mountain
WHERE height > 4000
```
### TUTORIAL D
```Rel
(Mountain WHERE height > 4000.0) {name}
```

## Question 3
List the provinces of Belgium (country code B) that have a population that doesn't exceed
one million; for each province, provide the name of the capital and the population.
### SQL
```SQL
SELECT capital, population
FROM Province
WHERE country = 'B' AND population < 1000000
```
### TUTORIAL D
```Rel
(Province WHERE country = 'B' AND population < 1000000) {capital, population}
```

## Question 4
List the names of all languages in the database. The result must be a
relation with one attribute called language
### SQL
```SQL
SELECT DISTINCT name
FROM Language
```
### TUTORIAL D
```Rel
(Language) {name} RENAME {name AS language}
```

## Question 5
For each country that has declared independence, give its full name and its independence date.
### SQL
```SQL
SELECT name, independence
FROM Country, Independence
WHERE Independence.country = Country.code
```
### TUTORIAL D
```Rel
(Independence JOIN Country WHERE country = code) {name, independence}
```

## Question 6
List all countries which are on two different continents. The result must be a relation
defined with 3-tuple attributes {name, percentage, continent} where name is the name of the country.
### SQL
```SQL
SELECT name, percentage, continent
FROM Country, Encompasses
WHERE percentage != 100 AND Country.code = Encompasses.country
```
### TUTORIAL D
```Rel
(Country JOIN Encompasses WHERE  NOT (percentage = 100.0) AND code = country) {name, percentage, continent}
```

## Question 7
List the Swiss mountains with a height between 4400 and 4500. The result must
be a relation composed of 2-tuples with attributes {mountain, height}.
### SQL
```SQL
SELECT name, height
FROM Mountain, GeoMountain
WHERE GeoMountain.country = 'CH'
  AND Mountain.name = GeoMountain.mountain
  AND height BETWEEN 4400 AND 4500
```
### TUTORIAL D
```Rel
(Mountain Join GeoMountain WHERE country = 'CH' AND name = mountain AND height > 4400.0 AND height < 4500.0) {mountain, height}
```

## Question 8
List all pairs of neighbouring countries. The result must be a relation composed of
2-tuples with attributes {name1, name2}, i.e., the full country names must be given.
(For instance, a result could be {Belgium, France}.) Make sure that every pair of
neighbouring countries is only included once.
### SQL
```SQL
SELECT C1.name, C2.name
FROM borders, Country C1, Country C2
WHERE country1 < country2
    AND country1 = C1.code
    AND country2 = C2.code
```
### TUTORIAL D
```Rel
((Borders WHERE country1 < country2)
	JOIN (Country RENAME {name as name1, code as country1})
	JOIN (Country {name, code}
	RENAME{name as name2, code as country2})) {name1, name2}
```

## Question 9
List the name of the countries that do not have mountains.
### SQL
```SQL
SELECT name
FROM Country
WHERE code NOT IN (
    SELECT country
    FROM GeoMountain)
```
### TUTORIAL D
```Rel
((Country {code}) NOT MATCHING (GeoMountain {country} RENAME {country as code})  
	JOIN (Country)) {name}
```

## Question 10
List all names that are shared by a province and a country. (Hint ":" Your Tutorial D
query may not have more than 50 characters.)
### SQL
```SQL
SELECT name
FROM Province
    INTERSECT
        SELECT name
        FROM Country
```
### TUTORIAL D
```Rel
(Province {name}) INTERSECT (Country {name})
```

## Question 11
List the full names of the countries that border the United States, as well as the
countries that border those bordering countries":" i.e., the output should contain
Mexico, but also Belize, which borders Mexico. The United States itself should
not be part of the output.
### SQL
```SQL
SELECT name
FROM Borders JOIN Country
WHERE (country1 IN (
                SELECT country1
                FROM Borders
                WHERE country2 = 'USA')
        OR country1 = 'USA')
        AND country2 != 'USA'
        AND code = country2
```
### TUTORIAL D
```Rel
((((Borders MATCHING ((Borders
			WHERE country2 = 'USA'
			OR country1 = 'USA' ) {country1}))
		WHERE country2 <> 'USA') {country2} )
		JOIN (Country RENAME {code as country2})) {name}
```

## Question 12
Count the number of different ethnic groups. The result must be a
relation composed by a 1-tuple attribute {cnt}.
### SQL
```SQL
SELECT count(
    DISTINCT name) AS cnt
FROM EthnicGroup
```
### TUTORIAL D
```Rel

```

## Question 13
List the names of the countries where the citizens speak at least 3 languages.
The result must be a relation composed of 2-tuples with attributes {name, cnt},
where name is the name of the country and cnt is the number of languages that citizens speak.
### SQL
```SQL
SELECT Country.name, COUNT(country) AS cnt
FROM Country, Language
WHERE code = country
GROUP BY Country.name
HAVING   COUNT(country) >= 3
```
### TUTORIAL D
```Rel

```

## Question 14
List the most prominent language(s) for each country; i.e.,
list for each country those languages for which the percentage is maximal.
You are not allowed to use SUMMARIZE. The result must be a relation composed
of 2-tuples with attributes {country, name}, where country is the code of
the country and name is the name of the language.
### SQL
```SQL
SELECT L1.country AS country, L1.name AS name
FROM Language L1
WHERE L1.percentage IN (
    SELECT MAX(percentage)
    FROM Language L2
    WHERE L1.country = L2.country)
```
### TUTORIAL D
```Rel

```

## Question 15
In the given database the Borders relation is symmetric":" if the country 1 (c1)
borders country 2 (c2), country 2 also borders country 1. One may wish to check
this property on a given database. Write a query that determines all tuples {c1, c2},
where c1 and c2 are the codes of the countries, for which the inverse direction is missing.
(Note":" on the given database, the output of this query should be empty; on a database
in which the Border relation is not symmetric, however, it should return the violating tuples.
Create a database yourself to check this.)
### SQL
```SQL
SELECT country1 AS c1, country2 AS c2
FROM Borders
EXCEPT
SELECT country2, country1
FROM Borders
```
### TUTORIAL D
```Rel

```
