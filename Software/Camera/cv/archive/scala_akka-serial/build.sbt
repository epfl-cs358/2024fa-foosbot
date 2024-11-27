scalaVersion := "2.13.15"

resolvers += "Akka library repository".at("https://repo.akka.io/maven")

libraryDependencies += "com.typesafe.akka" %% "akka-actor" % "2.10.0"

// For akka-serial
// main artifact
libraryDependencies += "ch.jodersky" %% "akka-serial-core" % "4.2.0"

// "fat" jar containing native libraries
libraryDependencies += "ch.jodersky" % "akka-serial-native" % "4.2.0" % "runtime"
