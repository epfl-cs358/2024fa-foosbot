scalaVersion := "2.11.8"

resolvers += "Akka library repository".at("https://repo.akka.io/maven")
libraryDependencies += "com.typesafe.akka" %% "akka-actor" % "2.10.0"

resolvers += Resolver.bintrayRepo("jarlakxen", "maven")
libraryDependencies += "com.github.jarlakxen" %% "reactive-serial" % "1.4"
