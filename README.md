====
Arduino LED Controller for FlexTech 108 LED Christmas Light Set
====

Extension of the code found at: http://seeedstudio.com/wiki/index.php?title=Twig_-_Chainable_RGB_LED
(page changed in January of 2013; version referenced:
  http://seeedstudio.com/wiki/index.php?title=Grove_-_Chainable_RGB_LED&oldid=18188
)

Controls an LED digitally addressable light sight made by FlexTech: http://www.gemmy.com/christmas/christmas-lighting/item/christmas-lighting/flextech-lightshow-multi-color-87081
(page no longer there)

Contains extensions to the original P9613 controller code from seedstudio (FlexTech light set uses P9618, but appears to be controllable by the P9613 code).

Additions (convenience functions) made to simplify controlling the LEDs indiviually, since the controller chips are originally meant to control 3 RGB LEDs, and in this light
set they are controlling two groups of 9 LEDs (first chip controls 9 Red & 9 Blue, second chip controls 9 Green & 9 Yellow; pattern repeats for total of 6 controller chips,
controlling 3 complete color groups).

Also added ability to send commands via serial (115200 baud), allowing control from a computer (or other TTL serial source).

