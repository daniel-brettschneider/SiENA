---
title: PrivADE
permalink: /algo-privade
---

PrivADE is a Privacy-Preserving Algorithm for Distributed Energy Management. It distinguishes between three different types of loads, which are handled separately in the algorithm: shiftable loads, adaptable loads and switchable loads. Privacy is implemented using homomorphic encryption. The following sections give a detailed description of PrivADE.

## Preliminaries
Households form a distributed system. A smart metering device exists, which is connected to all participating devices of the household. It also connects to a communication network and has the ability to communicate with a server, which acts as a registry and control unit for the distributed algorithm. The households do not have to trust the server, because it does not have any specific knowledge of single households. Communication-security-wise, public key encryption, e.g. RSA, is used to secure all communication. Key generation and distribution is done independently from the algorithm using standard methods.

Shiftable devices, e.g. a washing machine or drier, can be shifted to a later point in time when activated by the owner, regarding an according deadline. For example, a drier is activated in the morning and should be finished in the afternoon. The algorithm can choose the optimal time frame for an activation. The energy consumption of adaptable devices, e.g. a BS or an EV, can be adapted dynamically. For example an EV can dynamically feed into the grid to counteract a temporary power lack of a PV. As opposed to adaptable devices, switchable devices, e.g. a HP or CHP, can only be turned on or off. For example, a HP can be activated when a lot of PV power exists to fill a heat storage. Thus, heating power exists during the night, without the need of consuming energy.

## Data Aggregation using Homomorphic Encryption
Homomorphic encryption uses a public key encryption scheme. Operations performed on the ciphertext have a corresponding result on the plaintext. PrivADE uses additive operations, therefore, as opposed to a fully HES, a partial HES is chosen with additive capabilities. Several partial HESs exist: RSA, ElGamal, Paillier, etc. Paillier is chosen, because of its low message expansion overhead, acceptable encryption cost and efficient decryption cost. It supports additive and subtractive operations.

The Paillier cryptosystem is based on the intractability hypothesis of composite degree residuosity classes. Two large prime numbers \\( p \\) and \\( q \\) are chosen, where \\( \gcd(pq,(p-1)(q-1))=1 \\). Let \\( n=pq \\), \\( \lambda=lcm(p-1,q-1) \\) and \\( g \in \mathbb{Z}\_{n^2}^{\*} \\) be a random, where \\( n \\) divides the order of \\( g \\). The public key is \\( (n,g) \\) and the private key \\( \lambda \\). To encrypt a plaintext \\( m \in \mathbb{Z}\_n \\), \\( r \in \mathbb{Z}\_{n}^{\*} \\) is chosen randomly. The ciphertext \\( c \\) is computed as

$$
c = g^m \cdot r^n \bmod n^2.
$$

To decrypt \\( c \\), the function \\( L \\) is defined as \\( L(u)=\frac{u-1}{n} \\). The plaintext is computed as

$$
m = \frac{L(c^\lambda\bmod n^2)}{L(g^\lambda\bmod n^2)} \bmod n.
$$

The homomorphic capabilities of the Paillier cryptosystem allow for homomorphic addition and subtraction of plaintexts using an encryption function \\( \mathsf{E} \\) and a decryption function \\( \mathsf{D} \\):

$$
\mathsf{D}(\mathsf{E}(m_1) {\cdot} \mathsf{E}(m_2) \bmod n^2) = m_1 {+} m_2 \bmod n\\
\mathsf{D}(\mathsf{E}(m_1) {\cdot} {\mathsf{E}(m_2)^{-1}} \bmod n^2) = m_1 {-} m_2 \bmod n
$$

![net]({{ site.url }}{{ site.baseurl }}/assets/img/privade_net.png)
{: .half }

## Round 1 -- Gathering Data
To achieve privacy, PrivADE utilises homomorphic encryption for data aggregation. Participants are ordered in a virtual ring with the server as the starting point. The order of the ring can be determined independently beforehand. PrivADE is based on rounds. In the first round all necessary data is gathered, while preserving privacy. In general, the server \\( s \\) creates a data packet with several counters, encrypts it using the Paillier cryptosystem and sends it through the virtual ring. Thus, households \\( h \in H \\) are able to privately add their data and not even internal attackers gain any knowledge. Only the server can decrypt the data packet using its private key, but even then only has access to aggregated data.

Different types of data are needed to perform energy management using PrivADE. First, the current total consumption \\( \omega \\). Second, the adaption potential of all households. Third, the switching potential of all devices. The Paillier cryptosystem allows merging values \\( a \\) and \\( b \\) using the scheme

$$
\begin{aligned}
\mathsf{D}(\mathsf{E}(a \ll x) \cdot \mathsf{E}(b \ll x) \bmod n^2)\\ = (a \ll x) + (b \ll x) \bmod n,
\end{aligned}
$$

where \\( \ll \\) denotes a bit-shift operation and \\( x \\) a bit size. In general, it is recommended to use large prime numbers security wise. Thus, with \\( n \\) of 1024 bit, 128 counters of unsigned 8 bit can be used in a single operation. However, this does not apply to subtractions, but all listed values can be reduced to unsigned additive counters: The consumption \\( \omega_h\in\mathbb{Z} \\) of a household \\( h \\) is split into a positive \\( \omega^+ \\) and a negative counter \\( \omega^- \\).

The adaption potential of a household consists of an aggregation of its adaptable devices \\( D_h \\). An adaptable device \\( d\in D_h \\) is classified by two values: The capability of increasing its load by \\( \alpha_d \\) or decreasing it by \\( \beta_d \\). Thus, the adaption counters are defined by the possible increase \\( \alpha \\) in consumption of all households, the number \\( A \\) of households which can increase their consumption, the possible decrease \\( \beta \\) in consumption of all households and the corresponding number \\( B \\) of households. If a household can increase and/or decrease its consumption, \\( A \\) and \\( B \\) are incremented accordingly and

$$
\alpha_h = \sum_{d \in D_h} \alpha_{d} \text{ and } \beta_h = \sum_{d \in D_h} \beta_{d}.
$$

In conclusion, \\( \omega^+ \\), \\( \omega^- \\), \\( \alpha \\), \\( A \\), \\( \beta \\) and \\( B \\) form a data packet. The number space can be adapted to support differing amounts of participants. In this case 32 bit are used, to support unsigned integers.

To preserve privacy, switchable devices are classified in categories \\( \lambda\in\Lambda \\) to prohibit transmission of consumption data. With additional priorities \\( p\in P \\), a new element \\( \theta \in \Theta \\) arises, where \\( \Theta:=P\times\Lambda \\). For example, a set of categories \\( \Lambda_p \\) for a priority \\( p\in P \\) might be defined as

$$
	\left\{ \dots, -1000 W, -500 W, 0 W, 500 W, 1000 W, \dots \right\}.
$$

Thus, every category has a specific range, e.g. \\( [-1000,-500) \\), and counts switchable devices within this range. If a household has, for example, a CHP which can be turned on, it increments the counter in the category which corresponds to the electrical generation, e.g. \SI{-950}{W}. Depending on the charge level of an associated heat storage, a priority can be used. A CHP with a lower charge level should be turned on with a higher priority. In this scenario, a size of 8 bit is chosen for a category, so that 255 devices can be added.

The concatenation \\( \epsilon_h \\) of all counters resembles the data packet of a household in the first round.

$$
  \epsilon_h =\ \omega_h^+ \mathrel{\mathsf{or}} (\omega_h^- \ll 32)\\
  \mathrel{\mathsf{or}} \left(\alpha_h \ll 64\right) \mathrel{\mathsf{or}} \left(A_h \ll 96\right) \\
  \mathrel{\mathsf{or}} \left(\beta_h \ll 128\right) \mathrel{\mathsf{or}} \left(B_h \ll 160\right) \\
  \mathrel{\mathsf{or}} \left(\theta_{h,1} \ll 168\right) \mathrel{\mathsf{or}} \dots \\
  \mathrel{\mathsf{or}} \left(\theta_{h,\left|\Theta\right|} \ll (160+8\cdot\left|\Theta\right|) \right),
$$

where \\( \mathrel{\mathsf{or}} \\) denotes a bitwise or-operation. Depending on \\( \Theta \\), \\( \epsilon \\) has to be split into blocks. If 1024 bit are chosen for the Paillier cryptosystem, 104 categories are available in the first block. Therefore, only one block of data has to be encrypted. Increasing the number of categories also increases the number of encrypted blocks. \\( \epsilon \\) initialises as \\( \epsilon_0=0 \\) and encrypts as \\( \mathsf{E}(\epsilon_0) \\) with the public key \\( s_{pub} \\) of the server and a random value according to the Paillier cryptosystem. The server sends the data packet to the first household in the virtual overlay network, which aggregates and sends the data to the second one, etc. Non-adaptable households are concealed, because of the random part of Paillier. After the first round, the server \\( s \\) receives the value

$$
\epsilon_s = \left(\mathsf{E}(\epsilon_0) \cdot \prod_{h\in H} \mathsf{E}(\epsilon_h)\right) \bmod n^2.
$$

## Round \\( 2^+ \\) -- Switching and Adapting Loads
After the first round, the server determines whether any action is required. A load target \\( \mu \\) is defined, e.g. 50 kW for 100 households. Based on the current consumption and the potential of all switchable and adaptable loads, energy management is performed.

In a first step the goal is to stimulate switchable loads to keep the deviation from the load target in \\( [\mu-\alpha,\mu+\beta] \\). Thus, adaptable loads can further reduce the deviation in a second step. First, the server fulfils all categories and adds them to the total consumption. If the range is violated, categories are randomly removed beginning with the lowest priority until the defined range is reached or until there are no switchable categories any more. Thus, \\( \Theta'\subseteq\Theta \\) defines categories which have to be switched.
%This information is sent to all participants in the second round.

After determining switchable loads, the residual error is reduced by adaptable loads using the max-min fairness principle. If the households consume more power and some have the potential to decrease their consumption, the algorithm fairly distributes the error amongst the adaptable households. The same principle applies, if the consumption is below the load target \\( \mu \\).

$$
\nu =
\begin{cases}
\min(\alpha, \lvert \omega-\mu \rvert),	& \text{if } \omega \leq \mu\\
-\min(\beta, \lvert \omega-\mu \rvert),	& \text{else}
\end{cases}
$$

describes the possible adaption (positive: increase, negative: decrease). Thus, the load share for all adaptable participants is defined by

$$
\zeta =
\begin{cases}
\frac{\nu}{A}, & \text{if } \nu > 0\\
\frac{\nu}{B},  & \text{else.}
\end{cases}
$$

From the second round on \{\\( \zeta \\), \\( \Theta' \\), \\( \epsilon=(\delta\ll32)\mathrel{\mathsf{or}}\Delta \\)\} states a new data packet, where \\( \zeta \\) and \\( \Theta' \\) are static (\\( \Theta' \\) is only used in the second round) and \\( \epsilon \\) aggregates and encrypts remaining adaption potential \\( \Delta \\) and residual share \\( \delta \\) using the same scheme. Upon receiving the data packet, a household with switchable devices checks whether the according categories should be switched. Furthermore, an adaptable household determines whether it can fulfil the load share. If it can fulfil the share, it increases or decreases its consumption \\( \omega_h \\) depending on \\( \zeta \\). Remaining adaption potential is indicated by incrementing \\( \Delta \\). If it can only partially fulfil the share, the residual share \\( \zeta_{r} \\) is added to \\( \delta \\). Thus, the remaining deficit received by the server is

$$
\delta_s = \mathsf{E}(\delta_0) \cdot \prod_{h\in H} \mathsf{E}(\zeta_{h,r}).
$$

This procedure is repeated until the total consumption \\( \omega \\) observes acceptable limits or until no adaptable households remain.

![shift]({{ site.url }}{{ site.baseurl }}/assets/img/shift.png)

## Load Shifting
Shiftable Loads are excluded from the distributed algorithm on purpose to reduce the communication overhead. To guarantee reasonable results, loads can be shifted a whole day maximum. In previous versions of PrivADE, the expected future consumption has been sent through the virtual ring in the first round, so that households with shiftable loads could perform the load shifting. The optimal activation time is determined, for example, a valley in the expected consumption during the night, the corresponding load added to the expected consumption and sent on to the next household in the ring. Thus, some privacy is preserved, because the households perform necessary calculations by themselves and the expected consumption is aggregated. However, internal attackers can still analyse the data and gain knowledge by forming coalition attacks. Furthermore, since a base interval of one minute is used, the expected future consumption consists of 1440 individual values of e.g. each 32 bit. Thus, a large communicational and computational cost exists. Additionally, homomorphic encryption cannot be used to preserve privacy, because the households need access to the individual values of the expected consumption.

As a result, load shifting is performed by every household itself, only using a standard load profile (H0, Germany) without any communication. A household determines the cost of the activation of the device for each possible tick. A random solution of all solutions within acceptable range of the best one is chosen. This yields reasonable results without the need of spreading information. Thus, privacy is preserved using a stochastic approach.
