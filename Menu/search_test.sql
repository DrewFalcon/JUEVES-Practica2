
SELECT air.airport_code, 
FROM airports_data air 
JOIN flights ff ON f.arrival_airport = air.airport_code


GROUP BY, ff.scheduled_departure
ORDER BY ff.scheduled_departure