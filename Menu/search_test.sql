
WITH firstFLight AS (

    
    FROM airports_data d
    JOIN flights f ON f.departure_airport = d.airport_code
    LEFT JOIN airports_data t ON f.arrival_airport = t.airport_code



    WHERE d.airport_name = 'FROM'
    AND t.airport_name = 'TO'



)

SELECT vuelo1, *vuelo2
FROM airports_data d
JOIN 


WHERE d.airport_name = 'FROM'


--1. Del aereopuerto FROM se buscan todos los vuelos que empiezen por ese aereopuerto
--2. De cada uno de esos vuelos, se va a ver el aereopuerto al que llego y su fecha
--      2.1 Si el "scheduled arrival" dura mas de 24 horas entre el departure, no se selecciona
--      2.2 Si el arrival airport es el mismo que el aereopuerto TO, se selecciona para mostrar en la grafica
--3. Si el aereopuerto que llega NO es el aereopuerto TO, se repite el proceso con el segundo aereopuerto
--      3.1 DIferencia siendo que ahora, el limite de tiempo no es 24 horas, sino 24 horas menos la diferencia del primer vuelo
--      3.2 Otra diferencia, es que se utilizara TICKET_FLIGHTS en vez de solo el nuevo aereopuerto
--          3.2.1 Del vuelo original, se buscan los tickets que tengan un flight_id similar al vuelo original (solo deberia de haber uno?).
--          3.2.2 De esos ticket flights, se consigue su ticket_no (parte de TICKET, que puede contener multiples TIcket_flights, y multiples flights)
--          3.2.3 Del ticket_no conseguido, se buscan los ticket_flights cuyo flight_id tenga
--              3.2.3.1 Como aereopuerto de departue, el mismo que el segundo aereopuerto conseguido
--              3.2.3.2 El aereopuerto de arrival, el mismo que el aereopuerto TO pedido.
--              3.2.3.3 EL shceduled arrival, no genere un tiempo superior de 24 horas al sumarlo con el primer viaje
--          3.2.4 Los que cumplan los requisitos, mostrarlos en el SELECT como lista



--x. Aplicar las condiciones extra 
--      x.1 GROUP sera por vuelos, con sus listas de 2do vuelo si posible