package serial

import scala.io.StdIn
import akka.actor.ActorSystem
import akka.serial.{ Serial, SerialSettings, Parity }

object Main {
  /*
   *  Ask the user for an entry, if nothing is received `default` is returned.
   *
   *  @param label   Label of asked component
   *  @param default Default value to be returned
   */
  def ask(label: String, default: String) = {
    print(label + " [" + default.toString + "]: ")
    val in = StdIn.readLine()
    println("")
    if (in.isEmpty) default else in
  }

  def main(args: Array[String]): Unit = {
    val port     = ask("Device", "/dev/ttyUSB0")
    val baud     = ask("Baud rate", "115200").toInt
    val cs       = ask("Char size", "8").toInt
    val tsb      = ask("Use two stop bits", "false").toBoolean
    val parity   = Parity(ask("Parity (0=None, 1=Odd, 2=Even)", "0").toInt)
    val settings = SerialSettings(baud, cs, tsb, parity)

    println("Starting terminal system, enter :q to exit.")
    Serial.debug(true)
    val system   = ActorSystem("akka-serial")
    val terminal = system.actorOf(Terminal(port, settings), name = "terminal")
    system.registerOnTermination(println("Stopped terminal system."))
  }
}
