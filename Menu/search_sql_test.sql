
WITH RECURSIVE flight_paths AS (
    -- Vuelos directos (Caso base)
    SELECT 
        f.flight_id, f.departure_airport, f.arrival_airport, f.scheduled_departure, f.scheduled_arrival,
        ARRAY[f.flight_id] AS flight_chain, (f.scheduled_arrival - f.scheduled_departure) AS total_duration, 1 AS connection_count
    FROM flights f
    WHERE f.departure_airport = 'UUA'
    AND (f.scheduled_arrival - f.scheduled_departure) < INTERVAL '24 hours'
    
    UNION ALL
    
    -- Conexiones (Caso recursivo)
    SELECT 
        f2.flight_id,
        fp.departure_airport AS original_departure,
        f2.arrival_airport,
        fp.scheduled_departure AS original_departure_time,
        f2.scheduled_arrival,
        fp.flight_chain || f2.flight_id,
        (f2.scheduled_arrival - fp.scheduled_departure) AS total_duration,
        fp.connection_count + 1
    FROM flight_paths fp
    JOIN ticket_flights tf1 ON tf1.flight_id = fp.flight_id
    JOIN tickets t ON t.ticket_no = tf1.ticket_no
    JOIN ticket_flights tf2 ON tf2.ticket_no = t.ticket_no
    JOIN flights f2 ON f2.flight_id = tf2.flight_id
    WHERE 
        f2.departure_airport = fp.arrival_airport
        AND f2.scheduled_departure >= fp.scheduled_arrival  -- Después de la llegada anterior
        AND f2.scheduled_departure <= (fp.scheduled_arrival + INTERVAL '24 hours')  -- Dentro de 24 horas
        AND f2.flight_id <> ALL(fp.flight_chain)
        AND (f2.scheduled_arrival - fp.scheduled_departure) < INTERVAL '24 hours'
        AND fp.connection_count < 3  -- Límite de conexiones
)
SELECT DISTINCT
    fp.flight_chain,
    fp.departure_airport AS original_departure,
    fp.arrival_airport AS final_arrival,
    fp.scheduled_departure AS original_departure_time,
    fp.scheduled_arrival AS final_arrival_time,
    fp.total_duration,
    fp.connection_count
FROM flight_paths fp
WHERE 
    fp.arrival_airport = 'SVO'
ORDER BY fp.total_duration;