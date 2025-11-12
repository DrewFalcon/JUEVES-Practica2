WITH tickets_sin_boarding AS (
    SELECT 
        t.ticket_no,
        t.passenger_name,
        tf.flight_id,
        f.scheduled_departure,
        f.aircraft_code
    FROM tickets t
    JOIN ticket_flights tf ON t.ticket_no = tf.ticket_no
    JOIN flights f ON tf.flight_id = f.flight_id
    WHERE t.book_ref = ?
      AND NOT EXISTS (
          SELECT 1 
          FROM boarding_passes bp 
          WHERE bp.ticket_no = t.ticket_no 
            AND bp.flight_id = tf.flight_id
      )
),
asientos_disponibles AS (
    SELECT 
        tsb.ticket_no,
        tsb.flight_id,
        tsb.aircraft_code,
        (SELECT s.seat_no
         FROM seats s
         WHERE s.aircraft_code = tsb.aircraft_code
           AND NOT EXISTS (
               SELECT 1 
               FROM boarding_passes bp
               WHERE bp.flight_id = tsb.flight_id
                 AND bp.seat_no = s.seat_no
           )
         ORDER BY s.seat_no
         LIMIT 1) AS seat_no_disponible
    FROM tickets_sin_boarding tsb
)
SELECT 
    tsb.ticket_no,
    tsb.passenger_name,
    tsb.flight_id,
    tsb.scheduled_departure,
    tsb.aircraft_code,
    ad.seat_no_disponible
FROM tickets_sin_boarding tsb
JOIN asientos_disponibles ad ON tsb.ticket_no = ad.ticket_no 
    AND tsb.flight_id = ad.flight_id
WHERE ad.seat_no_disponible IS NOT NULL
ORDER BY tsb.ticket_no, tsb.flight_id;
