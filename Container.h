 #ifndef CONTAINER_H
 #define CONTAINER_H
 // verhindert das Programmteile mehrmals inkludiert werden
 
 // Container.h
 // 
 // UE Algorithmen und Datenstrukturen - SS 2015 Universitaet Wien
 // Container - Projekt
 // https://cewebs.cs.univie.ac.at/ADS/ss15/
 // Der container gibt das Interface vor das von allen konkreten Containerimplementierung erfüllt werden muss
 
 #include <iostream>
 #include <functional>
 
 enum Order { dontcare, ascending, descending };		//legt fest, in welcher Reihenfolge die Elemente des Containers zu bearbeiten sind
 
 class ContainerException : public std::exception {
 public:
   virtual const char* what() const noexcept override = 0;
 };
 
 template <typename E>
 class Container {
 public:
   Container& operator=(const Container&) = delete;		//Kopierzuweisung
   Container(const Container&) = delete;				//Kopierkonstruktor
   Container() = default;								//Konstruktor
 
   virtual ~Container() { }		//Virtueller Destruktor: Verhindert, dass bei Freigabe eines Objekts einer erbenden Klasse über einen Pointer oder eine Referenz auf die Basisklasse der Speicher nicht korrekt freigegeben wird.
    
   virtual void add(const E& e) { add(&e, 1); }		//Es wird hier bereits ein Verhalten in der Basisklasse vorgegeben. Kann in erbenden Klasse jedoch angepasst werden
   virtual void add(const E e[], size_t s) = 0;		//pure virtual function: konkrete Implementierung erfolgt nur in der erbenden Klasse
													//dies macht den Container zu einer abstrakten Klasse -> keine Instanzierung von Container möglich
 
   virtual void remove(const E& e) { remove(&e, 1); }
   virtual void remove(const E e[], size_t s) = 0;
 
   virtual bool member(const E& e) const = 0;
   virtual size_t size() const = 0;
   virtual bool empty() const { return size() == 0; }
 
   virtual size_t apply(std::function<void(const E&)> f, Order order = dontcare) const = 0;
 
   virtual E min() const = 0;
   virtual E max() const = 0;
  
   virtual std::ostream& print(std::ostream& o) const = 0;
 };
 
template <typename E>
inline std::ostream& operator<<(std::ostream& o, const Container<E>& c) { return c.print(o); }		//operator<< muss nur für die Basisklasse definiert werden. Die aufgerufene print-Methode ist virtuell.
																									//Es wird daher immer die Ausgabeoperation (print-Methode) der jeweils aktuell betroffenen Klasse aufgerufen. 
  
 template <typename E> inline size_t hashValue(const E& e) { return size_t(e); }
 template <typename E> inline double doubleValue(const E& e) { return double(e); }
 template <typename E> inline size_t ordinalValue(const E& e) { return size_t(e); }
 
 #endif //CONTAINER_H
