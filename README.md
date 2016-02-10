# pebble-polar-utc

It's a watchface based on [PolarDot](https://apps.getpebble.com/en_US/application/56a4c2bd11d0acf5c5000027?section=watchfaces)
but with some tweaks. It looks like this:

![polar-utc](https://cloud.githubusercontent.com/assets/833911/12937694/3f3e8720-cf79-11e5-96e2-a6fd539ff57b.png)


## Behavior
The watchface shows the local time hour in large text, followed by the current date, followed by the UTC hour. 

Minutes are represented by the arc around the outside. Every 5 minutes, the arc extends, with dots for each minute between.

When the battery is low (11-20%), the large hour text turns purplish and when it's very low (1-10%), the large hour text turns blue.

When the watch disconnects from bluetooth, it buzzes once, and the date turns blue.

## Support
Currently only Basalt (Pebble Time, not Round). I probably won't add support for other platforms, but I'm open to PRs, or you can fork the project.

## License
MIT
