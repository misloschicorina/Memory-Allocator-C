# Segregated Free List Memory Allocator

**Misloschi Alexandra Corina**  
**314 CA, 2023-2024**  

---

## Descriere

Pentru implementarea mea a alocatorului de memorie am folosit structura numită **segregated_free_lists**, ce conține:
- **vectorul de liste dublu înlănțuite**
- **dimensiunea vectorului de liste** (`size`)
- **numărul de bytes al listelor** (`bytes_per_list`)

Array-ul este de tip `linked_list`, o **listă dublu înlănțuită generică**, care păstrează:
- **head-ul** listei  
- **dimensiunea datelor** din noduri (`data_size`)  
- **dimensiunea listei** (`size`)  

Head-ul este de tip `nod`, iar **nodul** este generic și conține:
- **pointeri către** `next` și `prev`
- `void *data` → folosit pentru stocarea adresei nodului, deoarece în alocator/heap nu se vor pune alte date sau conținut

O a doua structură de date, utilizată pentru **blocurile de memorie alocate**, este `allocated_list`, o listă care păstrează:
- **nodul de tip** `block`
- **dimensiunea listei înlănțuite** (`size`)  

Structura `block` include:
- `void *data` → pentru orice tip de date
- **pointeri către** `next` și `prev` (blocurile anterioare și succesoare)
- **un câmp pentru adresa blocului**
- **`data_size`**, deoarece blocurile din `allocated_list` pot avea dimensiuni diferite  


---

## Funcționalități principale

### 🔹 **1. Inițializarea Heap-ului (`init_heap`)**
- Alocă memorie pentru **segregated_free_lists** și vectorul de liste.
- Creează `nr_of_lists` liste (dat ca parametru), cu dimensiuni **puteri ale lui 2**, începând cu `2^3`.
- Adaugă noduri în liste, iar adresele stocate în câmpul `data` respectă **dimensiunea putere de 2** și **consecutivitatea dorită**.

---

### 🔹 **2. Alocare memorie (`malloc_function`)**
- Se parcurge `segregated_free_lists` și se caută un **bloc exact** de dimensiunea `nr_bytes`.
- Dacă se găsește un astfel de bloc, acesta este **eliminat din lista sa** și adăugat în `allocated_list`.
- Dacă nu există un bloc exact, se caută un bloc **mai mare** care să poată fi **împărțit în două**:
  - O parte va fi de **dimensiunea cerută**.
  - Restul este **reintrodus în segregated_free_lists**.

---

### 🔹 **3. Eliberare memorie (`free_block`)**
- Se caută blocul de memorie cu **adresa dată** în `allocated_list`.
- Dacă blocul este găsit, este eliminat din listă, iar numărul de **free-uri** este incrementat.
- Dacă nu este găsit, se afișează:  
- Blocul eliberat este adăugat în **segregated_free_lists**, dacă există o listă compatibilă.
- Dacă lista **nu există**, se creează una nouă.

---

### 🔹 **4. Dump Heap (`dump_mem`)**
Afișează informații despre:
- **Heap** (`segregated_free_lists`)
- **Blocurile alocate** (`allocated_list`)
- **Statistici**:
- Numărul de **malloc-uri**
- Numărul de **free-uri**
- Numărul de **fragmentări**

---

### 🔹 **5. Scriere în memorie (`write`)**
- Verifică **lungimea sirului de date**.
- Caută blocurile **corespunzătoare adresei**.
- Dacă blocul **nu are spațiu suficient**, afișează:

---

### 🔹 **6. Citire din memorie (`read`)**
- Verifică existența **adresei specifice**.
- Dacă blocul există și este valid, **afișează datele**.
- În caz contrar, afișează eroarea:

---

### 🔹 **7. Distrugerea Heap-ului (`destroy`)**
- Eliberează memoria utilizată de:
- **Blocurile alocate** (`allocated_list`)
- **Listele segregate** (`segregated_free_lists`)

---

## Funcții auxiliare

Pentru optimizare, am implementat următoarele **funcții ajutătoare**:

- `create_list` → Creează o listă dublu înlănțuită generică.
- `cmp_addr` → Compară două adrese de memorie și returnează:
  - `1` dacă prima adresă este mai mare
  - `-1` dacă a doua adresă este mai mare
  - `0` dacă adresele sunt egale
- `add_node` → Adaugă un nod nou într-o listă astfel încât **adresele să rămână ordonate crescător**.
- `remove_nth_node` → Șterge al **n-lea nod** dintr-o listă dublu înlănțuită.
- `add_block` → Adaugă un bloc în `allocated_list`, în funcție de **adresa** primită ca parametru.
- `sort_address` → Sortează nodurile din aceeași listă **după adresă**.
- `sort_lists` → Sortează **listele din vector** după dimensiune.
- `add_linked_list` → Adaugă o **nouă listă** în vectorul de liste.
- `remove_block` → Șterge un **bloc existent** în lista-alocator.
- `print_sf_list` → Afișează, în formatul cerut la `dump memory`, nodurile din heap (`segregated_free_lists`), alături de dimensiunea acestora.
