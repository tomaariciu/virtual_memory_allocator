**Nume: Ariciu Toma**
**Grupă: 312CAb**

## Tema 1 - Segregated Free Lists

### Descriere:

* Programul este un alocator de memorie virtuala. Este mentinut un array de liste dublu inlantuite, care retin adresele de memorie ale blocurilor neocupate de aceeasi dimensiune.
De asemenea, blocurile de memorie alocate sunt mentinute intr-o lista dublu inlantuita. Toate listele sunt sortate dupa adresa.
* Array-ul are inca de la initializarea marimea celui mai mare bloc de memorie. Astfel, majoritatea listelor sunt initial goale.
* Pentru fiecare bloc este retinuta adresa corespunzatoare acestuia, marimea, un array de charuri care reprezinta alocarea efectiva a memoriei, precum si un id dat la initializare, care
ajuta la reconstituirea blocurilor dupa fragmentare.
* Cea mai importanta functie a programului este cea de lower_bound, care primeste ca parametrii o adresa si o lista si intoarce nodul din lista cu cea mai mare adresa care este mai mica decat cea cautata.
* Reconstituirea se face brut. La eliberarea unui bloc de memorie, se cauta vecinii din stanga si din dreapta printre blocurile neocupate. Daca este gasit vreun vecin, unim cele doua blocuri.
	
### Comentarii asupra temei:

* Pozitia 0 a array-ului contine intotdeauna o lista goala, folosind memorie degeaba. Totusi, codul este mult mai citibil astfel.
