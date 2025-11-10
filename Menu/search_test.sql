WITH RECURSIVE flight_paths AS (
    -- Vuelos directos
    SELECT 
        f.flight_id, f.departure_airport, f.arrival_airport, f.scheduled_departure, f.scheduled_arrival,
        ARRAY[f.flight_id] AS flight_chain, (f.scheduled_arrival - f.scheduled_departure) AS total_duration, 
        0 AS connection_count,
        -- Calcular asientos libres para vuelo directo
        (SELECT COUNT(*) FROM seats s WHERE s.aircraft_code = f.aircraft_code) 
        - 
        (SELECT COUNT(*) FROM boarding_passes bp WHERE bp.flight_id = f.flight_id) AS free_seats
    FROM flights f
    WHERE f.departure_airport = ?
      AND f.arrival_airport = ?
      AND DATE(f.scheduled_departure) = ?  -- Filtro por fecha
      AND (f.scheduled_arrival - f.scheduled_departure) < INTERVAL '24 hours'
      -- Filtrar vuelos con asientos disponibles
      AND ((SELECT COUNT(*) FROM seats s WHERE s.aircraft_code = f.aircraft_code) 
           - 
           (SELECT COUNT(*) FROM boarding_passes bp WHERE bp.flight_id = f.flight_id)) > 0
    
    UNION ALL
    
    -- Vuelos con conexión (hasta 1 transbordo)
    SELECT 
        f2.flight_id,
        fp.departure_airport AS original_departure,
        f2.arrival_airport,
        fp.scheduled_departure AS original_departure_time,
        f2.scheduled_arrival,
        fp.flight_chain || f2.flight_id,
        (f2.scheduled_arrival - fp.scheduled_departure) AS total_duration,
        fp.connection_count + 1,
        -- Tomar el MÍNIMO de asientos libres entre los vuelos
        LEAST(
            fp.free_seats,
            (SELECT COUNT(*) FROM seats s WHERE s.aircraft_code = f2.aircraft_code) 
            - 
            (SELECT COUNT(*) FROM boarding_passes bp WHERE bp.flight_id = f2.flight_id)
        ) AS free_seats
    FROM flight_paths fp
    JOIN flights f1 ON f1.flight_id = fp.flight_chain[array_length(fp.flight_chain, 1)]
    JOIN ticket_flights tf1 ON tf1.flight_id = f1.flight_id
    JOIN tickets t ON t.ticket_no = tf1.ticket_no
    JOIN ticket_flights tf2 ON tf2.ticket_no = t.ticket_no
    JOIN flights f2 ON f2.flight_id = tf2.flight_id
    WHERE 
        f2.departure_airport = f1.arrival_airport
        AND f2.scheduled_departure >= f1.scheduled_arrival 
        AND f2.scheduled_departure <= (f1.scheduled_arrival + INTERVAL '24 hours') 
        AND f2.flight_id <> ALL(fp.flight_chain)
        AND (f2.scheduled_arrival - fp.scheduled_departure) < INTERVAL '24 hours'
        AND fp.connection_count < 1  -- Solo 1 transbordo máximo
        -- Filtrar que el segundo vuelo tenga asientos disponibles
        AND ((SELECT COUNT(*) FROM seats s WHERE s.aircraft_code = f2.aircraft_code) 
             - 
             (SELECT COUNT(*) FROM boarding_passes bp WHERE bp.flight_id = f2.flight_id)) > 0
)
SELECT DISTINCT
    fp.flight_chain,
    fp.departure_airport AS original_departure,
    fp.arrival_airport AS final_arrival,
    fp.scheduled_departure AS original_departure_time,
    fp.scheduled_arrival AS final_arrival_time,
    fp.total_duration,
    fp.connection_count,
    fp.free_seats AS min_free_seats
    
FROM flight_paths fp
WHERE fp.free_seats > 0  -- Asegurar que hay asientos disponibles
ORDER BY fp.total_duration, fp.free_seats DESC;

--WITH firstFLight AS (

    
    --FROM airports_data d
    --JOIN flights f ON f.departure_airport = d.airport_code
    --LEFT JOIN airports_data t ON f.arrival_airport = t.airport_code



  --  WHERE d.airport_name = 'FROM'
  --  AND t.airport_name = 'TO'



--)

--SELECT vuelo1, *vuelo2
--FROM airports_data d
--JOIN 


--WHERE d.airport_name = 'FROM'


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